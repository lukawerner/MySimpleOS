#ifndef READY_QUEUE_H
#define READY_QUEUE_H
typedef struct ReadyQueue ReadyQueue;
ReadyQueue *ready_queue_create();
int *ready_queue_enqueue(PCB* pcb, ReadyQueue *queue);
void ready_queue_destroy(ReadyQueue *queue);
#endif