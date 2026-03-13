#include "program.h"
#include <string.h>
#include "shellmemory.h"
#include <stdio.h>
#include <stdlib.h>
#include "config.h"
#include <pthread.h>

extern pthread_mutex_t shellmemory_lock;

Program *program_table[MAX_PROGRAM];
int program_table_size = 0;

typedef struct Program {
    char *name;
    int base;
    int bounds;
    int pcb_pointing;
    int table_idx;
} Program;

Program *program_create(char *name, int base, int bounds) {
    Program* p = malloc(sizeof(Program));
    p->name = strdup(name); 
    p->base = base;
    p->bounds = bounds;
    p->pcb_pointing = 1;
    program_table[program_table_size] = p;
    program_table_size++;
    return p; 
}

int program_destroy(Program *p) {
    if (p->pcb_pointing != 0) return 1;
    if (p->name == NULL) return 1;
    if (p == NULL) return 1;

    prog_mem_free(p->base, p->bounds);
    free(p->name);
    free(p);
    return 0;
}

int program_get_bounds(Program* p) {
    return p->bounds;
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

int program_get_base(Program *p) {
    return p->base;
}


int load_program(Program *p) {
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
        pthread_mutex_lock(&shellmemory_lock);
        int start_idx = prog_mem_alloc(script_length);  // returns the start index, or -1 if it fails
    

    if (start_idx == -1) {
        printf("Program memory is full, can't allocate space\n");
        pthread_mutex_unlock(&shellmemory_lock);
        fclose(f);
        return 1;
    }
    rewind(f);

    for (int idx = start_idx; fgets(line, MAX_LINE_LENGTH, f) != NULL; idx++) {
        prog_write_line(idx, line);
    }

    pthread_mutex_unlock(&shellmemory_lock);
    fclose(f);
    p->base = start_idx;
    p->bounds = script_length;
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
        program_table[i] = program_table[i+1];
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