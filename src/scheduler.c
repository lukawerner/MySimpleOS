#include "scheduler.h"
#include "pcb.h"
#include "policies.h"
#include "readyqueue.h"
#include "shell.h"
#include "shellmemory.h"
#include <linux/limits.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "program.h"
#include "paging.h"
#include "lru.h"

extern pthread_mutex_t shellmemory_lock;
extern int multithreaded_mode;
extern pthread_mutex_t interpreter_lock;

int create_pcb_and_enqueue(char *script, ReadyQueue *queue, Policy *policy) {
    Program *prog;  
    if (!program_already_exists(script)) {
        prog = program_create(script);
        if (init_load_program(prog)) return 1;
    }
    else {
        prog = find_program_in_table(script);
        if (prog == NULL) return 1; 
    }
    PCB *new_pcb = pcb_create(prog); 
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
    int prog_length = pcb_get_program_size(process); 
    return pcb_get_pc(process) == prog_length;
}

int exec_program(PCB *process, ReadyQueue *queue, Policy *policy) { 
    int prog_length = pcb_get_program_size(process);
    int pc;
    int errorCode = 0;
    int lines_executed = 0;

    while (((pc = pcb_get_pc(process)) != prog_length) && (lines_executed != policy->job_length)) {
        pthread_mutex_lock(&shellmemory_lock);
        int address = pcb_get_physical_address(process);
        if (address == -1) {
            pthread_mutex_unlock(&shellmemory_lock);
            int errorCode = handle_page_fault(process);
            if (errorCode) exit(1);
            return errorCode;
        }
        //printf("address: %d\n", address); 
        char *curr_command = prog_read_line_unlocked(address);
        int frame_number = pcb_get_frame_number(process);
        update_mru(frame_number);
        
        pthread_mutex_unlock(&shellmemory_lock);

        if (multithreaded_mode) {
            pthread_mutex_lock(&interpreter_lock);
        }        
        errorCode = parseLine(curr_command);

        if (multithreaded_mode) {
            pthread_mutex_unlock(&interpreter_lock);
        }

        free(curr_command);

        if (errorCode) {
            printf("Process couldn't execute properly\n");
            exit(1);
            return errorCode;
        }
        pcb_increment_pc(process);
        lines_executed++;
    }

    return errorCode;
}

