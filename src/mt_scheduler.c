#include "mt_scheduler.h"
#include <pthread.h>
#include "readyqueue.h"
#include "policies.h"
#include <stdlib.h>
#include <stdio.h>
#include "scheduler.h"

pthread_cond_t queue_not_empty;
pthread_mutex_t ready_queue_lock;
int threads_initialized = 0;
int thread_shutdown = 0;

pthread_t worker[THREAD_NUMBER];


int run_multithreaded_scheduler(ReadyQueue *queue, Policy *policy) {    // supports fcfs, and RR
    if (!threads_initialized) {
        int errCode = 0;    

        WorkerArgs* arg = malloc(sizeof(WorkerArgs));
        arg->policy = policy;
        arg->queue = queue;

        pthread_mutex_init(&ready_queue_lock, NULL);
        pthread_cond_init(&queue_not_empty, NULL);

        
        for (int i=0; i<THREAD_NUMBER; i++) {
            errCode = pthread_create(&worker[i], NULL, worker_scheduler, arg);
            if (errCode) {
                printf("Couldn't create thread %d\n", i);
                return errCode;
            }
        }
        threads_initialized = 1;
        for (int i=0; i<THREAD_NUMBER; i++) {
            pthread_join(worker[i], NULL);
        }
        exit(0);
    }
    else {
        pthread_cond_broadcast(&queue_not_empty); // if already initialized threads, only option is that exec has been nested called
    }                                             // ReadyQueue now has new PCBs
}

void *worker_scheduler(void* arg) {
    WorkerArgs *arguments = (WorkerArgs *) arg;
    ReadyQueue *queue = arguments->queue;
    Policy *policy = arguments->policy;
    int errorCode = 0;

    while (1) {
        pthread_mutex_lock(&ready_queue_lock);
        while (queue->head == NULL && !thread_shutdown) {
            pthread_cond_wait(&queue_not_empty, &ready_queue_lock);
        }
        if (thread_shutdown && queue->head == NULL) {
            pthread_mutex_unlock(&ready_queue_lock);
            return NULL;
        }
        PCB *process = ready_queue_dequeue(queue);
        pthread_mutex_unlock(&ready_queue_lock);

        errorCode = exec_program(process, queue, policy);

        if (errorCode) {
            return NULL;
        }
        if (process_completed(process)) {
            pthread_mutex_lock(&ready_queue_lock);
            pcb_destroy(process);
            pthread_mutex_unlock(&ready_queue_lock);
        }
        else {
            pthread_mutex_lock(&ready_queue_lock);
            errorCode = ready_queue_enqueue(process, queue, policy);
            pthread_cond_signal(&queue_not_empty);
            pthread_mutex_unlock(&ready_queue_lock);

            if (errorCode) {
                printf("Couldn't enqueue uncompleted process\n");
                return NULL;
            }
        }
    }
}

void handle_quit() {
    if (!threads_initialized) {
        return;
    }
    pthread_mutex_lock(&ready_queue_lock);
    thread_shutdown = 1;
    pthread_cond_broadcast(&queue_not_empty);
    pthread_mutex_unlock(&ready_queue_lock);
    return;
}

