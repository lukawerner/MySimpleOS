#ifndef READY_QUEUE_H
#define READY_QUEUE_H

#include "pcb.h"
#include "scheduler.h"

typedef struct ReadyQueue {
    PCB *head;
    PCB *tail;
} ReadyQueue;
//extern ReadyQueue ready_queue;
void ready_queue_init(ReadyQueue *queue);
int ready_queue_enqueue(PCB* pcb, ReadyQueue *queue, Policy *policy);
PCB* ready_queue_dequeue(ReadyQueue *queue);

#endif