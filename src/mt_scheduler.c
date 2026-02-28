#include "mt_scheduler.h"
#include "policies.h"
#include "readyqueue.h"
#include "scheduler.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

pthread_cond_t queue_not_empty = PTHREAD_COND_INITIALIZER;
pthread_mutex_t ready_queue_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t interpreter_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t shellmemory_lock = PTHREAD_MUTEX_INITIALIZER;

int threads_initialized = 0;
int thread_shutdown = 0;
static WorkerArgs *worker_args = NULL;
static Policy shared_policy;
int request_quit = 0;
static int workers_active = 0;

pthread_t worker[THREAD_NUMBER];

int run_multithreaded_scheduler(ReadyQueue *queue, Policy *policy) {  // supports fcfs, and RR
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

        for (int i = 0; i < THREAD_NUMBER; i++) {
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
        pthread_cond_broadcast(&queue_not_empty);  // if already initialized threads, only option is
                                                   // that exec has been nested called
        pthread_mutex_unlock(&ready_queue_lock);   // ReadyQueue has new PCBs, broadcast to threads so
                                                   // they start working
    }
    return 0;
}

void *worker_scheduler(void *arg) {
    WorkerArgs *arguments = (WorkerArgs *)arg;
    ReadyQueue *queue = arguments->queue;
    Policy *policy = arguments->policy;
    int errorCode = 0;

    while (1) {
        pthread_mutex_lock(&ready_queue_lock);
        while (queue->head == NULL && !thread_shutdown) {            // queue empty and threads haven't seen a quit command yet
            pthread_cond_wait(&queue_not_empty, &ready_queue_lock);  // we wait
        }
        if (thread_shutdown && queue->head == NULL) {  // if queue empty and quit command seen, worker's job is done
            pthread_mutex_unlock(&ready_queue_lock);
            return NULL;
        }
        PCB *process = ready_queue_dequeue(queue);
        workers_active++;
        pthread_mutex_unlock(&ready_queue_lock);

        errorCode = exec_program(process, queue, policy);

        if (errorCode) {
            return NULL;
        }
        pthread_mutex_lock(&ready_queue_lock);
        if (process_completed(process)) {
            pcb_destroy(process);
        } 
        else {
            errorCode = ready_queue_enqueue(process, queue, policy);

            pthread_cond_signal(&queue_not_empty);

            if (errorCode) {
                printf("Couldn't enqueue uncompleted process\n");
                return NULL;
            }
        }
        workers_active--;
        if (queue->head == NULL && workers_active == 0) pthread_cond_broadcast(&queue_not_empty);  // we wake up sleeping main thread in handle_quit()
        pthread_mutex_unlock(&ready_queue_lock);
    }
}

void handle_quit() {  // can only be called by the main thread
    pthread_mutex_lock(&ready_queue_lock);
    if (!threads_initialized) {
        pthread_mutex_unlock(&ready_queue_lock);
        return;
    }

    while (ready_queue.head != NULL || workers_active > 0) {  // we stall until all workers finish and all PCBs are executed
        pthread_cond_wait(&queue_not_empty, &ready_queue_lock);
    }

    thread_shutdown = 1;                       // the thread which sees the quit command sets the shutdown flag
    pthread_cond_broadcast(&queue_not_empty);  // and broadcasts to the others to wake up if waiting,
                                               // allowing them to exit
    pthread_mutex_unlock(&ready_queue_lock);

    for (int i = 0; i < THREAD_NUMBER; i++) {  //
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
