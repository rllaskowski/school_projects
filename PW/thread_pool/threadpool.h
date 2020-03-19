#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <stddef.h>
#include <pthread.h>
#include <stdbool.h>

typedef struct runnable {
    void (*function)(void *, size_t);
    void *arg;
    size_t argsz;
} runnable_t;


typedef struct task_queue {
    runnable_t task;
    struct task_queue *next;
} task_queue_t;


typedef struct thread_pool {
    struct thread_pool *next;
    struct thread_pool *prev;
    task_queue_t *task_first;
    task_queue_t *task_last;
    pthread_t *threads;
    pthread_attr_t threads_attr;
    size_t num_threads;
    pthread_cond_t task_cond;
    pthread_mutex_t thread_mutex;
    bool running;

} thread_pool_t;

int thread_pool_init(thread_pool_t *pool, size_t pool_size);

void thread_pool_destroy(thread_pool_t *pool);

int defer(thread_pool_t *pool, runnable_t runnable);

#endif
