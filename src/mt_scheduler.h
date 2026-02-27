#ifndef MT_SCHEDULER_H
#define MT_SCHEDULER_H
#include <pthread.h>
#define THREAD_NUMBER 2

typedef struct ReadyQueue ReadyQueue;
typedef struct Policy Policy;

typedef struct WorkerArgs{
    Policy *policy;
    ReadyQueue *queue;
} WorkerArgs;

extern pthread_cond_t queue_not_empty;
extern pthread_mutex_t ready_queue_lock;
extern int thread_shutdown;
extern int threads_initialized;


int run_multithreaded_scheduler(ReadyQueue *queue, Policy *policy);
void *worker_scheduler(void* arg);
void handle_quit();



#endif
