#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "pcb.h"

typedef struct ReadyQueue ReadyQueue;
extern ReadyQueue ready_queue;
typedef struct Policy Policy;

int create_pcb_and_enqueue(char *script, ReadyQueue *queue, Policy *policy);
int run_scheduler(ReadyQueue *queue, Policy *policy);
int process_completed(PCB *process);
int exec_program(PCB *process, ReadyQueue *queue, Policy *policy);

#endif
