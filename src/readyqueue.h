#ifndef READY_QUEUE_H
#define READY_QUEUE_H

#include "pcb.h"
typedef struct ReadyQueue {
    PCB *head;
    PCB *tail;
} ReadyQueue;
extern ReadyQueue ready_queue;
void ready_queue_init(ReadyQueue *queue);
int ready_queue_enqueue(PCB* pcb, ReadyQueue *queue);
PCB* ready_queue_pop(ReadyQueue *queue);
PCB* ready_queue_dequeue(ReadyQueue *queue);
int ready_queue_destroy(ReadyQueue *queue);
PCB *ready_queue_get_head(ReadyQueue *queue);

#endif