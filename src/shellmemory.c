#include "shellmemory.h"
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include "config.h"
#include "program.h"
#include <stdlib.h>
#include <stdio.h>
#include "program.h"

struct memory_struct {
    char *var;
    char *value;
};
static char *frame_store[FRAME_STORE_SIZE];
struct memory_struct shellmemory[MEM_SIZE];
extern pthread_mutex_t shellmemory_lock;
extern int multithreaded_mode;

// Shell memory functions

void mem_init() {
    int i;
    for (i = 0; i < MEM_SIZE; i++) {
        shellmemory[i].var = "none";
        shellmemory[i].value = "none";
    }
}

// Set key value pair
void mem_set_value(char *var_in, char *value_in) {
    int i;

    for (i = 0; i < MEM_SIZE; i++) {
        if (strcmp(shellmemory[i].var, var_in) == 0) {
            shellmemory[i].value = strdup(value_in);
            return;
        }
    }

    // Value does not exist, need to find a free spot.
    for (i = 0; i < MEM_SIZE; i++) {
        if (strcmp(shellmemory[i].var, "none") == 0) {
            shellmemory[i].var = strdup(var_in);
            shellmemory[i].value = strdup(value_in);
            return;
        }
    }

    return;
}

// get value based on input key
char *mem_get_value(char *var_in) {
    int i;

    for (i = 0; i < MEM_SIZE; i++) {
        if (strcmp(shellmemory[i].var, var_in) == 0) {
            return strdup(shellmemory[i].value);
        }
    }
    return "Variable does not exist";
}

void frame_store_init() {
    for (int i = 0; i < FRAME_STORE_SIZE; i++) frame_store[i] = NULL;
}



int alloc_frame() {
    int start_idx = -1;          
    start_idx = search_free_frame(0, FRAME_STORE_SIZE-1);

    if (start_idx == -1) {
        return start_idx;
    }
    return start_idx/FRAME_SIZE;
}

void store_frame(int frame_number, char *script[], int script_length, int page_number) {
    //printf("store_frame_arguments: frame_number %d, script_length %d, page_number %d\n", frame_number, script_length, page_number);
    for (int i = 0, VA = page_number*FRAME_SIZE; VA<script_length && i < FRAME_SIZE ; i++, VA++) {
        int PA = frame_number * FRAME_SIZE + i;
        prog_write_line(PA, script[VA]);
    }
}


int search_free_frame(int start_idx, int end_idx) {
    if (start_idx >= end_idx) {
        return -1;
    }
    int potential_idx = (rand() % (end_idx-start_idx)) + start_idx;
    potential_idx = (potential_idx/FRAME_SIZE)*FRAME_SIZE; 
     
    if (frame_store[potential_idx] != NULL) {
        int next_potential_idx = potential_idx + FRAME_SIZE;
        potential_idx = search_free_frame(start_idx, potential_idx); 
        if (potential_idx != -1) return potential_idx;
        else {
            potential_idx = search_free_frame(next_potential_idx, end_idx);
        }
    }
    return potential_idx;


}

void prog_write_line(int idx, const char *line) { frame_store[idx] = strdup(line); }

char *prog_read_line(int idx) {
    pthread_mutex_lock(&shellmemory_lock);
    char *line = prog_read_line_unlocked(idx);
    pthread_mutex_unlock(&shellmemory_lock);
    return line;
}

char *prog_read_line_unlocked(int idx) {
    return strdup(frame_store[idx]);
}

void mem_free_frame(int frame_idx) {
    for (int i = 0; i < FRAME_SIZE; i++) {
        free(frame_store[frame_idx + i]);
        frame_store[frame_idx + i] = NULL;
    }
    }

void prog_mem_free(Program *p) {
    pthread_mutex_lock(&shellmemory_lock);
    int num_of_pages = program_get_num_of_frames(p);
    for (int i = 0; i<num_of_pages; i++) {
        int frame = program_get_frame(p, i);
        mem_free_frame(frame*FRAME_SIZE);
    } 
    pthread_mutex_unlock(&shellmemory_lock);
} 
