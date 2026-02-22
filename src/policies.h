#ifndef ENQUEUE_FUNCTIONS_H
#define ENQUEUE_FUNCTIONS_H
#include "readyqueue.h"
#include "pcb.h"
int fcfs_enqueue(PCB* pcb, ReadyQueue *queue, Policy *policy);
int sjf_enqueue(PCB* pcb, ReadyQueue *queue, Policy *policy);
void age_queue(ReadyQueue *queue);
int aging_and_score_is_smallest(PCB* pcb, ReadyQueue* queue, Policy* policy);
#endif