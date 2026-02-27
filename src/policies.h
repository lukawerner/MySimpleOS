#ifndef ENQUEUE_FUNCTIONS_H
#define ENQUEUE_FUNCTIONS_H
#include "readyqueue.h"
#include "pcb.h"


typedef struct Policy {
    int job_length;     // -1 indicates non preemptive
    int (*enqueue_function)(PCB* pcb, ReadyQueue* queue, struct Policy *policy);
    int (*get_metric_function) (PCB *pcb);                  // function p* to select a comparison metric for sjf_enqueue
                                                            //  (aging/non-aging = program_size/job_length_score)
} Policy;                            

Policy *parse_policy(const char *policy_string);
int fcfs_enqueue(PCB* pcb, ReadyQueue *queue, Policy *policy);
int sjf_enqueue(PCB* pcb, ReadyQueue *queue, Policy *policy);
void age_queue(ReadyQueue *queue);
int aging_and_score_is_smallest(PCB* pcb, ReadyQueue* queue, Policy* policy);
#endif