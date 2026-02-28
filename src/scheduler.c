#include "scheduler.h"
#include "pcb.h"
#include "policies.h"
#include "readyqueue.h"
#include "shell.h"
#include "shellmemory.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern pthread_mutex_t shellmemory_lock;
extern int multithreaded_mode;
extern pthread_mutex_t interpreter_lock;

int create_pcb_and_enqueue(char *script, ReadyQueue *queue, Policy *policy) {
    PCB *new_pcb = load_program(script);
    if (new_pcb == NULL) {
        printf("Couldn't create a PCB for new process %s\n", script);
        return 1;
    }
    return ready_queue_enqueue(new_pcb, queue, policy);
}

int run_scheduler(ReadyQueue *queue, Policy *policy) {
    int dequeue_allowed = 1;
    PCB *process = NULL;

    while ((queue->head != NULL) && (queue->tail != NULL)) {
        if (dequeue_allowed) {
            process = ready_queue_dequeue(queue);
        }

        dequeue_allowed = 0;

        int errorCode = exec_program(process, queue, policy);
        age_queue(queue);

        // program not done, job length reached
        if (!process_completed(process)) {
            if (aging_and_score_is_smallest(process, queue, policy)) {
                continue;
            }

            errorCode = ready_queue_enqueue(process, queue, policy);
            if (errorCode) {
                printf("Couldn't enqueue uncompleted process\n");
                return errorCode;
            }
        } 
        else {
            pcb_destroy(process);   // program done, we free the pcb and program from memory
        }

        dequeue_allowed = 1;
    }

    return 0;
}

int process_completed(PCB *process) {
    int prog_end_idx = pcb_get_memory_idx(process) + pcb_get_program_size(process);
    return pcb_get_pc(process) == prog_end_idx;
}

int exec_program(PCB *process, ReadyQueue *queue, Policy *policy) {
    int prog_end_idx = pcb_get_memory_idx(process) + pcb_get_program_size(process);
    int pc;
    int errorCode = 0;
    int lines_executed = 0;

    while (((pc = pcb_get_pc(process)) != prog_end_idx) && (lines_executed != policy->job_length)) {
        char *curr_command = prog_read_line(pc);

        if (multithreaded_mode) {
            pthread_mutex_lock(&interpreter_lock);
        }

        errorCode = parseInput(curr_command);

        if (multithreaded_mode) {
            pthread_mutex_unlock(&interpreter_lock);
        }

        free(curr_command);

        if (errorCode) {
            printf("Process couldn't execute properly\n");
            return errorCode;
        }
        pcb_increment_pc(process);
        lines_executed++;
    }

    return errorCode;
}

PCB *load_program(char *script) {
    FILE *f = fopen(script, "rt");

    if (f == NULL) {
        return NULL;
    }

    char line[MAX_LINE_LENGTH];
    int script_length = 0;

    while (fgets(line, MAX_LINE_LENGTH, f) != NULL) {
        script_length++;
    }

    if (script_length == 0) {
        printf("Script is empty\n");
        fclose(f);
        return NULL;
    }

    pthread_mutex_lock(&shellmemory_lock);
    int start_idx = prog_mem_alloc(script_length);  // returns the start index, or -1 if it fails

    if (start_idx == -1) {
        printf("Program memory is full, can't allocate space\n");
        pthread_mutex_unlock(&shellmemory_lock);
        fclose(f);
        return NULL;
    }
    rewind(f);

    for (int idx = start_idx; fgets(line, MAX_LINE_LENGTH, f) != NULL; idx++) {
        prog_write_line(idx, line);
    }

    pthread_mutex_unlock(&shellmemory_lock);
    fclose(f);
    return pcb_create(start_idx, script_length);
}
