#include "scheduler.h"
#include "shellmemory.h"
#include <stdio.h>
#include "pcb.h"
#include "readyqueue.h"
#include "shell.h"
#include <string.h>
#include <stdlib.h>

Policy *parse_policy(const char *policy_string) {
    Policy *new_policy = malloc(sizeof(Policy));
    if (strcmp(policy_string, "FCFS") == 0) {
        new_policy->job_length = -1;
        new_policy->enqueue_function = ready_queue_enqueue_fcfs;
    }
    else {
        free(new_policy);
        printf("Invalid policy input\n");
        return NULL;
    }
    return new_policy;
}

int create_pcb_and_enqueue(char *script, ReadyQueue *queue, Policy *policy) {
    PCB* new_pcb = load_program(script);
    if (new_pcb == NULL) {
        printf("Couldn't create a PCB for new process %s\n", script);
        return 1;
    }
    return policy->enqueue_function(new_pcb, queue);
}

int scheduler(ReadyQueue *queue, Policy *policy) {
    while ((queue->head != NULL) && (queue->tail != NULL)) {
        PCB *process = ready_queue_dequeue(queue);
        int start_idx = pcb_get_memory_idx(process);
        int prog_size = pcb_get_program_size(process);
        int pc;
        int errorCode;
        while ((pc = pcb_get_pc(process)) != (start_idx + prog_size)) {
            const char *curr_command = prog_read_line(pc);
            errorCode = parseInput(curr_command);
            if (errorCode) {
                printf("Process couldn't execute properly\n");
                return errorCode;
            }
            pcb_increment_pc(process);
        }
        pcb_destroy(process);
    }
    return 0;
}

PCB *load_program(char *script) {
    FILE *f = fopen(script, "rt");
    if (f == NULL) {
        return NULL;
    }

    char line[MAX_LINE_LENGTH];
    int script_length = 0;
    int idx = 0;
    int prog_idx = 0;

    while (fgets(line, MAX_LINE_LENGTH, f) != NULL) {
        script_length++;
    }
    if (script_length == 0) {
        printf("Script is empty\n");
        fclose(f);
        return NULL;
    }
    int start_idx = prog_mem_alloc(script_length);
    idx = start_idx;
    if (idx == -1) {
        printf("Program memory is full, can't allocate space\n");
        return NULL;
    }
    rewind(f);
    while (fgets(line, MAX_LINE_LENGTH, f) != NULL) {
        prog_write_line(idx, line);
        idx++;
    }
    fclose(f);
    PCB *pcb = pcb_create(start_idx, script_length);
    return pcb;
}