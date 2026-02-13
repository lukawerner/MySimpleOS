#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "shellmemory.h"

struct memory_struct {
    char *var;
    char *value;
};
static char *program_memory[PROGRAM_MEM_SIZE];
struct memory_struct shellmemory[MEM_SIZE];

// Helper functions
int match(char *model, char *var) {
    int i, len = strlen(var), matchCount = 0;
    for (i = 0; i < len; i++) {
        if (model[i] == var[i]) matchCount++;
    }
    if (matchCount == len) {
        return 1;
    } else return 0;
}

// Shell memory functions

void mem_init(){
    int i;
    for (i = 0; i < MEM_SIZE; i++){		
        shellmemory[i].var   = "none";
        shellmemory[i].value = "none";
    }
}

// Set key value pair
void mem_set_value(char *var_in, char *value_in) {
    int i;

    for (i = 0; i < MEM_SIZE; i++){
        if (strcmp(shellmemory[i].var, var_in) == 0){
            shellmemory[i].value = strdup(value_in);
            return;
        } 
    }

    //Value does not exist, need to find a free spot.
    for (i = 0; i < MEM_SIZE; i++){
        if (strcmp(shellmemory[i].var, "none") == 0){
            shellmemory[i].var   = strdup(var_in);
            shellmemory[i].value = strdup(value_in);
            return;
        } 
    }

    return;
}

//get value based on input key
char *mem_get_value(char *var_in) {
    int i;

    for (i = 0; i < MEM_SIZE; i++){
        if (strcmp(shellmemory[i].var, var_in) == 0){
            return strdup(shellmemory[i].value);
        } 
    }
    return "Variable does not exist";
}

void prog_mem_init() {
    for (int i = 0; i<PROGRAM_MEM_SIZE; i++) {
        program_memory[i] = NULL;
    }
}

int prog_mem_alloc(int size) { // allocates a contiguous block of size size inside program_memory
    int start_idx = -1;        // returns the start index, or -1 if it fails
    int block_available = 1;

    for (int i = 0; i < PROGRAM_MEM_SIZE - size; i++) {
        if (program_memory[i] != NULL) { // while slots are taken, we iterate
            continue;                    // over the array until we find an empty slot
        }
        // empty slot found
        start_idx = i;                   
        for (int j = i + 1; j < size; j++) {
            if (program_memory[j] != NULL) { // if the block can't be contiguous,
                block_available = 0;        //  we set the flag to false
                break;
            }
        }

        if (block_available) { // if the flag kept being true after the inner loop
            return start_idx;  // a contiguous block of size size was available
        }
        else {
            block_available = 1;
        }
    }
    return start_idx;
}

void prog_write_line(int idx, const char *line) {
    strdup(line, program_memory[idx]);
}

const char *prog_read_line(int idx) {
    return program_memory[idx];
}

void prog_mem_free(int start_idx, int size) {
    for (int i = 0; i < size; i++) {
        free(program_memory[start_idx + i]);
        program_memory[start_idx + i] = NULL;
    }
}