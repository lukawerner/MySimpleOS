#include "paging.h"
#include "pcb.h"
#include "program.h"
#include <stdio.h>
#include <stdlib.h>
#include "config.h"
#include "shellmemory.h"
#include "lru.h"
#include <pthread.h>

extern pthread_mutex_t shellmemory_lock;

int handle_page_fault(PCB *process) { 
    Program *program = pcb_get_program(process);
    int missing_page = pcb_get_pc(process)/FRAME_SIZE;
    int frame_store_full = load_program_page(program, missing_page);

    if (frame_store_full) {  
        if (evict_lru_frame()) return 1;
        load_program_page(program, missing_page); 
    }
    else {
        printf("Page fault!\n");
    }
    return 0;
}

int find_frame_in_prog_page_table(Program *program, int frame_number) {
    int *page_table = program_get_frames_idx(program);
    int table_length = program_get_num_of_frames(program);
    for (int i = 0; i<table_length; i++) {
        if (page_table[i] == frame_number) {
            return i;
        }
    }
    return -1;

}

Program *find_victim_program(int frame_number) {
    for (int i=0; i<program_table_size;i++) {
        Program *curr = program_table[i];
        int page_number = find_frame_in_prog_page_table(curr, frame_number);
        if (page_number != -1) {
            print_victim_lines(curr, page_number); 
            return curr;
        }
    }
    return NULL;
}

int evict_program_frame(Program *p, int frame_idx) { 
    int frame_number = frame_idx / FRAME_SIZE;
    int page_number = find_frame_in_prog_page_table(p, frame_number);
    int* page_table = program_get_frames_idx(p);
    int table_length = program_get_num_of_frames(p);

    if (program_update_page_table_entry(p, page_number, -1)) return 1;
    program_dec_pages_stored(p);
    mem_free_frame(frame_idx);
    return 0;
}

int evict_random_frame() {
    int victim_idx  = ((rand() % FRAME_STORE_SIZE)/FRAME_SIZE)*FRAME_SIZE;
    int victim_frame_num = victim_idx / FRAME_SIZE;
    Program *victim_prog = find_victim_program(victim_frame_num);
    if (victim_prog == NULL) {
        printf("Warning: Couldn't evict frame in memory at idx=%d because it wasn't allocated\n", victim_idx);
        return 1;
    }
    if (evict_program_frame(victim_prog, victim_idx)) {
        return 1;
    }
    return 0;
}

int evict_lru_frame() {
    pthread_mutex_lock(&shellmemory_lock);
    int victim_frame_num = get_lru_and_reorder();
    int victim_idx = victim_frame_num * FRAME_SIZE;
    Program *victim_prog = find_victim_program(victim_frame_num);
    if (victim_prog == NULL) {
        printf("Warning: Couldn't evict frame in memory at idx=%d because it wasn't allocated\n", victim_idx);
        pthread_mutex_unlock(&shellmemory_lock);
        return 1;
    }
    if (evict_program_frame(victim_prog, victim_idx)) {
        pthread_mutex_unlock(&shellmemory_lock);
        return 1;
    }
    pthread_mutex_unlock(&shellmemory_lock);
    return 0;
}

int print_victim_lines(Program *p, int page_num) {
    int lines_idx = page_num * FRAME_SIZE;
    const char *victim_name = program_get_name(p);
    char line[MAX_LINE_LENGTH];
    FILE *f = fopen(victim_name, "r");
    if (f == NULL) return 1; 
    printf("Page fault! Victim page contents:\n\n");
    int curr_idx = 0;
    int lines_printed = 0;
    while (fgets(line, MAX_LINE_LENGTH, f) != NULL) {
        if (lines_printed >= FRAME_SIZE) break;
        if (curr_idx >= lines_idx) {
            printf("%s", line);
            lines_printed++;
        }
        curr_idx++;
    }
    printf("\nEnd of victim page contents.\n");
    fclose(f);
    return 0;
}