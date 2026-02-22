#ifndef SCHEDULER_H
#define SCHEDULER_H
#define MAX_LINE_LENGTH 100
#include "pcb.h"

typedef struct ReadyQueue ReadyQueue;
extern ReadyQueue ready_queue;

typedef struct Policy {
    int job_length;     // -1 indicates non preemptive
    int (*enqueue_function)(PCB* pcb, ReadyQueue* queue, struct Policy *policy);
    int (*get_metric_function) (PCB *pcb);                  // function p* to select a comparison metric for sjf_enqueue
                                                            //  (aging/non-aging = program_size/job_length_score)
} Policy;                            

Policy *parse_policy(const char *policy_string);
int create_pcb_and_enqueue(char *script, ReadyQueue *queue, Policy *policy);
int run_scheduler(ReadyQueue *queue, Policy *policy);
PCB *load_program(char *script);

#endif