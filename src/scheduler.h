#ifndef SCHEDULER_H
#define SCHEDULER_H
#define MAX_LINE_LENGTH 100
#include "pcb.h"
#include "readyqueue.h"

typedef struct Policy {
    int job_length;     // -1 indicates non preemptive
    int (*enqueue_function)(PCB* pcb, ReadyQueue* queue);
} Policy;

Policy *parse_policy(const char *policy_string);
int create_pcb_and_enqueue(char *script, ReadyQueue *queue, Policy *policy);
int scheduler(ReadyQueue *queue, Policy *policy);
PCB *load_program(char *script);

#endif