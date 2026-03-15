#include "pcb.h"
#include "shellmemory.h"
#include <stdlib.h>
#include <sys/types.h>
#include "program.h"
#include <stdio.h>
#include "config.h"

static pid_t pid_tracker = 1;

typedef struct PCB {
    pid_t pid;
    Program *program;
    int pc;
    int job_length_score;
    PCB *next;
    int *page_table;
    int page_table_size;
    int backgroundModeOn;  // set to 1 if we are in background mode and pcb is a batch script, else 0
} PCB;

PCB *pcb_create(Program *program) {
    PCB *pcb = malloc(sizeof(PCB));
    pcb->pid = pid_tracker;
    pid_tracker++;
    pcb->program = program; 
    pcb->pc = 0;
    pcb->job_length_score = program_get_length(program);
    program_inc_pcb_pointing(program);
    pcb->next = NULL;
    pcb->page_table_size = program_get_num_of_pages(program);
    pcb->page_table = program_get_frames_idx(program);
    
    pcb->backgroundModeOn = 0;
    return pcb;
}

int pcb_get_frame_number(PCB* pcb) {
    if (pcb->page_table == NULL) {
        printf("Page table uninitialized for process: %s\n", program_get_name(pcb->program));
        exit(1);
    }
    int page_number = pcb->pc/FRAME_SIZE;
    int page = pcb->page_table[page_number];
    return page;
}

int pcb_get_page_offset(PCB *pcb) { 
    return pcb->pc%FRAME_SIZE;
}

int pcb_get_physical_address(PCB *pcb) {
    int frame_number = pcb_get_frame_number(pcb);
    int offset = pcb_get_page_offset(pcb);
    return frame_number + offset;
}

void pcb_toggle_background_mode(PCB *pcb) { pcb->backgroundModeOn = !(pcb->backgroundModeOn); }

int pcb_get_background_mode(PCB *pcb) { return pcb->backgroundModeOn; }

void pcb_destroy(PCB *pcb) {
    int pcb_count = program_get_pcb_pointing(pcb->program);
    program_dec_pcb_pointing(pcb->program);
    if (pcb_count == 1) {
        if (program_destroy(pcb->program)) {
            printf("Error: Another process using freed executable: %s\n", program_get_name(pcb->program));
            exit(1);
        }
    }
    if (pcb != NULL) free(pcb);
}

void pcb_increment_pc(PCB *pcb) { 
    pcb->pc++; 
}

void pcb_set_next(PCB *pcb, PCB *next) { 
    pcb->next = next;
}

PCB *pcb_get_next(PCB *pcb) {
    return pcb->next; 
}

int pcb_get_pc(PCB *pcb) {
    return pcb->pc; 
}

Program *pcb_get_program(PCB *pcb) {
    return pcb->program;
}

int pcb_get_program_size(PCB *pcb) {
    return program_get_length(pcb->program);
}

void pcb_decrement_job_length_score(PCB *pcb) {
    if (pcb->job_length_score > 0) {
        pcb->job_length_score--;
    }
}

int pcb_get_job_length_score(PCB *pcb) {
    return pcb->job_length_score;
}
