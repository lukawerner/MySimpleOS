#include "program.h"
#include <string.h>
#include "shellmemory.h"
#include <stdio.h>
#include <stdlib.h>
#include "config.h"
#include <pthread.h>
#include "helper.h"

extern pthread_mutex_t shellmemory_lock;

Program *program_table[MAX_PROGRAM];
int program_table_size = 0;

typedef struct Program {
    char *name;
    int pcb_pointing;
    int table_idx;
    int num_of_pages;
    int *frames_idx;
    int length;
} Program;

Program *program_create(char *name) {
    Program* p = malloc(sizeof(Program));
    p->name = strdup(name); 
    p->pcb_pointing = 0;
    program_table[program_table_size] = p;
    p->table_idx = program_table_size;
    program_table_size++;
    p->length = 0;
    return p; 
}
int program_load_frames(Program * p, char** lines, int n_frames) {
    p->num_of_pages = n_frames; 
    p->frames_idx = malloc(sizeof(int) * n_frames);
    if (p->frames_idx == NULL) {
        printf("Frame pointers intializer failed for %s\n", p->name);
        exit(1);
    }
    for (int i = 0; i<p->num_of_pages; i++) {
        int store_idx = alloc_frame(i);
        p->frames_idx[i] = store_idx;
        store_frame(store_idx, lines, p->length, i*FRAME_SIZE); 
    }
    return 0;
}
int program_destroy(Program *p) {
    if (p == NULL) return 1;
    if (p->pcb_pointing != 0) return 1;
    if (p->name == NULL) return 1; 

    prog_mem_free(p);
    remove_prog_from_table(p);

    free(p->name);
    free(p->frames_idx);
    free(p);
    return 0;
}


int program_get_pcb_pointing(Program *p) {
    return p->pcb_pointing;
}

void program_inc_pcb_pointing(Program *p) {
    p->pcb_pointing++;
}

void program_dec_pcb_pointing(Program *p) {
    p->pcb_pointing--;
}

const char* program_get_name(Program *p) {
    return p->name;
}

int program_get_num_of_pages(Program *p) {
    return p->num_of_pages;
}

int program_get_frame(Program *p, int idx) { 
    if (p->frames_idx == NULL) {
        printf("Frames weren't allocated for program: %s\n", p->name);
        exit(1);
    }
    return p->frames_idx[idx];
}

int *program_get_frames_idx(Program *p) {
    return p->frames_idx;
}

int program_set_length(Program *p) {
     FILE *f = fopen(p->name, "rt");

    if (f == NULL) {
        return 1;    
    }
   
    char line[MAX_LINE_LENGTH];
    int script_length = 0;

    while (fgets(line, MAX_LINE_LENGTH, f) != NULL) {
        script_length++;
    }

    if (script_length == 0) {
        printf("Script is empty\n");
        fclose(f);
        return 1;
    }
    p->length = script_length;
    fclose(f);
    return 0;
}

void background_program_set_length(Program *p, int script_length) {
    p->length = script_length;
}

int program_get_length(Program *p) {
    return p->length;
}

int convert_length_to_pages(int length) {
    if (length % FRAME_SIZE == 0) {
        return length/FRAME_SIZE;
    }
    else {
        return length/FRAME_SIZE + 1;
    }
}

int load_program(Program *p) { 
    program_set_length(p);
    int script_length = p->length;
    FILE *f = fopen(p->name, "rt");

    if (f == NULL) {
        return 1;
    }

    char** program_lines = malloc(sizeof(char*) * script_length);
    char line[MAX_LINE_LENGTH];
    for (int i = 0; fgets(line, MAX_LINE_LENGTH, f) != NULL; i++) {
        program_lines[i] = strdup(line);
    }

    int n_frames = convert_length_to_pages(script_length);
    pthread_mutex_lock(&shellmemory_lock);
    
    program_load_frames(p, program_lines, n_frames);

    pthread_mutex_unlock(&shellmemory_lock);
    fclose(f);
    free_array(program_lines, script_length);
    return 0;
}

Program *find_program_in_table(char *name) {
    for (int i=0; i<program_table_size; i++) {
        if (strcmp(program_table[i]->name, name) == 0) {
            return program_table[i];
        }
    }
    return NULL;
}

int remove_prog_from_table(Program *p) {
    int idx = p->table_idx;
    if (p != program_table[idx]) {
        return 1;
    }
    for (int i = p->table_idx; i<program_table_size - 1; i++) {
        Program* left_shifted = program_table[i+1];
        program_table[i] = left_shifted;
        left_shifted->table_idx--;
        
    }
    program_table_size--;
    p->table_idx = -1;
    return 0;
}


int program_already_exists(char *name) {
    int found = 0;
    for (int i = 0; i<program_table_size; i++) {
        if (strcmp(name, program_table[i]->name) == 0) {
            found = 1;
            break;
        }
    }
    return found;
}