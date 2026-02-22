#include "policies.h"
#include <stdio.h>

int fcfs_enqueue(PCB* pcb, ReadyQueue *queue, Policy *policy) {
    PCB *prev_tail = queue->tail;
    pcb_set_next(prev_tail, pcb);
    queue->tail = pcb;
    return 0;
}

int sjf_enqueue(PCB* pcb, ReadyQueue *queue, Policy *policy) {             // FOR SJF AND SJF AGING
    int (*get_sorting_metric)(PCB*) = policy->get_metric_function;              // function pointer to either select program size
                                                                           // or job length score as metric, depending on policy
    int pcb_metric = get_sorting_metric(pcb);

    if (pcb_metric < get_sorting_metric(queue->head)) {                                                          
        pcb_set_next(pcb, queue->head);                                 // pcb becomes head
        queue->head = pcb;
    }
    else if (pcb_metric >= get_sorting_metric(queue->tail)) {
        pcb_set_next(queue->tail, pcb);                                 // pcb becomes tail
        queue->tail = pcb;
    }
    else {                                                              // else pcb will sit in between,
        PCB *prev = queue->head;                                        // ensuring more than 1 node in list                         
        PCB *next = pcb_get_next(prev);                 
        int next_metric = get_sorting_metric(next);

        while (pcb_metric >= next_metric) {                             // we find the index where to insert
            prev = next;                                                // by comparing metrics
            next = pcb_get_next(next);
            if (next != NULL) {
                next_metric = get_sorting_metric(next);
            }
            
        }

        pcb_set_next(prev, pcb);
        pcb_set_next(pcb, next);
    }
    return 0;
}

void age_queue(ReadyQueue *queue) {
    PCB *curr = queue->head;
    while (curr != NULL) {
        pcb_decrement_job_length_score(curr);
        curr = pcb_get_next(curr);
    }
}

int aging_and_score_is_smallest(PCB* pcb, ReadyQueue* queue, Policy* policy) {
    if (queue->head == NULL) { // queue is empty
        return 0;
    }
    int aging = 0;
    int smallest_score = 0;
    int (*get_sorting_metric)(PCB*) = policy->get_metric_function;

    if (get_sorting_metric == pcb_get_job_length_score) {
        aging = 1;
    }

    if (pcb_get_job_length_score(pcb) <= pcb_get_job_length_score(queue->head)) {
        smallest_score = 1;
    }

    return (aging && smallest_score);
}