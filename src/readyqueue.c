#include "readyqueue.h"
#include "pcb.h"
#include <stdlib.h>
#include <stdio.h>

ReadyQueue ready_queue;

void ready_queue_init(ReadyQueue *queue) {
    if (queue == NULL) return;
    queue->head = NULL;
    queue->tail = NULL;
}

int ready_queue_enqueue(PCB* pcb, ReadyQueue *queue) {
    if (pcb == NULL || queue == NULL) {
        printf("Input arguments are NULL\n");
        return 1;
    }
    else if (pcb_get_next(pcb) != NULL) {
        printf("PCB already in ready queue\n");
        return 1;
    }
    else if (queue->tail == NULL || queue->head == NULL) { //empty list
        queue->head = pcb;
        queue->tail = pcb;
    }
    else {
        PCB *prev_tail = queue->tail;
        pcb_set_next(prev_tail, pcb);
        queue->tail = pcb;
    }
    return 0;
}
PCB* ready_queue_dequeue(ReadyQueue *queue) {
    if (queue == NULL) {
        printf("Ready queue doesn't exist\n");
        return NULL;
    }
    else if ((queue->tail == NULL) || (queue->head == NULL)) {
        printf("Can't dequeue from empty list\n");
        return NULL;
    }
    PCB *prev_head = queue->head;
    PCB *new_head = pcb_get_next(prev_head);
    if (new_head == NULL) { // only one element was in queue
        queue->tail = NULL;
    }
    queue->head = new_head;
    pcb_set_next(prev_head, NULL);
    return prev_head;
}

PCB* ready_queue_pop(ReadyQueue *queue) {
    if (queue->tail == queue->head) {
        printf("Ready queue is empty\n");
        return NULL;
    }
    else if ((queue->tail == NULL) || (queue->head == NULL)) {
        printf("Ready queue's tail or head fields are empty\n");
        return NULL;
    }
    PCB *tmp = queue->head;
    while (pcb_get_next(tmp) != queue->tail) {
        tmp = pcb_get_next(tmp);
    }
    queue->tail = tmp;
    tmp = pcb_get_next(tmp);
    pcb_set_next(queue->tail, NULL);
    return tmp;
}

int ready_queue_destroy(ReadyQueue *queue) {
    if (pcb_get_next(queue->head) == NULL) {
        printf("Ready queue is already empty\n");
        return 1;
    }
    while (queue->head != queue->tail) {
        PCB *tmp = pcb_get_next(queue->head);
        pcb_destroy(queue->head);
        queue->head = tmp;
    }
    free(queue);
    return 0;
}

PCB *ready_queue_get_head(ReadyQueue *queue) {
    return queue->head;
}
