#ifndef SCHEDULER_H
#define SCHEDULER_H
#define MAX_LINE_LENGTH 100
#include "pcb.h"
#include "readyqueue.h"
int create_process(char *script, ReadyQueue *queue);
int schedule_fcfs(ReadyQueue *queue);
PCB *load_program(char *script);
#endif