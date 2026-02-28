#include "mt_scheduler.h"
#include <pthread.h>
#include "readyqueue.h"
#include "policies.h"
#include <stdlib.h>
#include <stdio.h>
#include "scheduler.h"


pthread_cond_t queue_not_empty = PTHREAD_COND_INITIALIZER;
pthread_mutex_t ready_queue_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t interpreter_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t shellmemory_lock = PTHREAD_MUTEX_INITIALIZER;

int threads_initialized = 0;
int thread_shutdown = 0;
static WorkerArgs *worker_args = NULL;
static Policy shared_policy;
int request_quit = 0;

pthread_t worker[THREAD_NUMBER];


int run_multithreaded_scheduler(ReadyQueue *queue, Policy *policy) {    // supports fcfs, and RR
    if (!threads_initialized) {
        int errCode = 0;    

        worker_args = malloc(sizeof(WorkerArgs));
        if (worker_args == NULL) {
            printf("Couldn't allocate worker args\n");
            return 1;
        }

        shared_policy = *policy;
        worker_args->policy = &shared_policy;
        worker_args->queue = queue;
        thread_shutdown = 0;
        request_quit = 0;

        /*pthread_mutex_init(&ready_queue_lock, NULL);
        pthread_cond_init(&queue_not_empty, NULL);
        pthread_mutex_init(&interpreter_lock, NULL);
        pthread_mutex_init(&shellmemory_lock, NULL);*/

        for (int i=0; i<THREAD_NUMBER; i++) {
            errCode = pthread_create(&worker[i], NULL, worker_scheduler, worker_args);
            if (errCode) {
                printf("Couldn't create thread %d\n", i);
                return errCode;
            }
        }
        pthread_mutex_lock(&ready_queue_lock);
        threads_initialized = 1;
        pthread_cond_broadcast(&queue_not_empty);
        pthread_mutex_unlock(&ready_queue_lock);
        return 0;
    }
    else {
        pthread_mutex_lock(&ready_queue_lock);
        pthread_cond_broadcast(&queue_not_empty); // if already initialized threads, only option is that exec has been nested called
        pthread_mutex_unlock(&ready_queue_lock);
    }                                             // ReadyQueue now has new PCBs
    return 0;                                             
}

void *worker_scheduler(void* arg) {
    WorkerArgs *arguments = (WorkerArgs *) arg;
    ReadyQueue *queue = arguments->queue;
    Policy *policy = arguments->policy;
    int errorCode = 0;

    while (1) {
        pthread_mutex_lock(&ready_queue_lock);
        while (queue->head == NULL && !thread_shutdown) { // queue empty and threads haven't seen a quit command yet
            pthread_cond_wait(&queue_not_empty, &ready_queue_lock); // we wait
        }
        if (thread_shutdown && queue->head == NULL) { // if queue empty and quit command seen, worker's job is done
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

void handle_quit() {    // can only be called by the main thread
    pthread_mutex_lock(&ready_queue_lock);
    if (!threads_initialized) {
        pthread_mutex_unlock(&ready_queue_lock);
        return;
    }
    thread_shutdown = 1;                      // the thread which sees the quit command sets the shutdown flag
    pthread_cond_broadcast(&queue_not_empty); // and broadcasts to the others to wake up if waiting, allowing them to exit
    pthread_mutex_unlock(&ready_queue_lock);

    for (int i=0; i<THREAD_NUMBER; i++) { // 
        pthread_join(worker[i], NULL);
    }
    pthread_mutex_lock(&ready_queue_lock);
    threads_initialized = 0;
    thread_shutdown = 0;
    pthread_mutex_unlock(&ready_queue_lock);
    free(worker_args);
    worker_args = NULL;
    return;
}
