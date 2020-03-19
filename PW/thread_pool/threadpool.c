#include <pthread.h>
#include <signal.h>
#include <memory.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "threadpool.h"

static thread_pool_t *pools = NULL;
static pthread_mutex_t pools_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t signal_mutex = PTHREAD_MUTEX_INITIALIZER;
static bool signal_fired = false;


bool push_task(thread_pool_t *pool, runnable_t task) {
    if (pool != NULL) {
        task_queue_t *new_task = malloc(sizeof(task_queue_t));
        new_task->task = task;
        new_task->next = NULL;
        if (pool->task_first == NULL) {
            pool->task_first = new_task;
            pool->task_last = new_task;
        } else {
            pool->task_last->next = new_task;
            pool->task_last = new_task;
        }
        return true;
    }
    return false;
}

task_queue_t * pull_task(thread_pool_t *pool) {
    if (pool == NULL) {
        return NULL;
    } else {
        if (pool->task_first == NULL) {
            return NULL;
        } else {
            task_queue_t *task = pool->task_first;
            pool->task_first = pool->task_first->next;
            return task;
        }
    }
}

void * thread_work(void *pool_ptr) {
    thread_pool_t *pool = (thread_pool_t *)pool_ptr;
    task_queue_t *task;

    sigset_t block_mask;
    sigemptyset (&block_mask);
    sigaddset(&block_mask, SIGINT);  
    sigprocmask(SIG_BLOCK, &block_mask, 0);

    while (true) {
        pthread_mutex_lock(&(pool->thread_mutex));
        if (!pool->running && pool->task_first == NULL) {
            pthread_mutex_unlock(&(pool->thread_mutex));
            break;
        }

        while (pool->task_first == NULL && pool->running) {
            pthread_cond_wait(&(pool->task_cond), &(pool->thread_mutex));
        }
    
        if (pool->task_first != NULL) {
            task = pull_task(pool);
        } else {
            task = NULL;
        }
        pthread_mutex_unlock(&(pool->thread_mutex));

        if (task != NULL) {
            task->task.function(task->task.arg, task->task.argsz);
            free(task);
        }
    }
    pthread_cond_signal(&(pool->task_cond));
    
    return NULL;
}     

void pool_cleanup(thread_pool_t *pool) {
    if (pool != NULL) {
        pthread_cond_destroy(&(pool->task_cond));
        pthread_mutex_destroy(&(pool->thread_mutex));

        free(pool->threads);
        pthread_attr_destroy(&(pool->threads_attr));
    }

}

void thread_pool_destroy(struct thread_pool *pool) {
    if (pool != NULL) {
        pthread_mutex_lock(&pools_mutex);
        if (pool->next != NULL) {
            pool->next->prev = pool->prev;
        }
        if (pool->prev != NULL) {
            pool->prev->next = pool->next;
        }
        pthread_mutex_unlock(&pools_mutex);

        if (pool->running) {
            pthread_mutex_lock(&(pool->thread_mutex));
            pool->running = false;
            pthread_cond_signal(&(pool->task_cond));
            pthread_mutex_unlock(&(pool->thread_mutex));

            for (size_t i = 0; i < pool->num_threads; i++) {
                pthread_join(pool->threads[i], NULL);
            }
           
            pool_cleanup(pool);
        }
    }
}


void sig_handler() {
    signal_fired = true;
    pthread_mutex_lock(&signal_mutex);
    thread_pool_t *pool = pools;

    while (pool != NULL) {
        thread_pool_t *tmp = pool->next;
        thread_pool_destroy(pool);
        pool = tmp;
    } 

    pthread_mutex_unlock(&signal_mutex);

    pthread_exit(0);
}

int thread_pool_init(thread_pool_t *pool, size_t num_threads) {
    signal(SIGINT, sig_handler);
    if (pool != NULL && !signal_fired) {
        pthread_mutex_lock(&pools_mutex);
        
        pool->running = true;
        pool->task_first = NULL;
        pool->task_last = NULL;
        pool->num_threads = num_threads;
        pool->next = pools;
        pool->prev = NULL;
        
        pools = pool;
        
        pthread_mutex_init(&(pool->thread_mutex), NULL);
        pthread_cond_init(&(pool->task_cond), NULL);

        pool->threads = malloc(num_threads*sizeof(*pool->threads));

        pthread_attr_init(&pool->threads_attr);

        for (size_t i = 0; i < num_threads; i++) {
            pthread_create(&(pool->threads[i]), &pool->threads_attr, thread_work, pool);
        }
        pthread_mutex_unlock(&pools_mutex);
        return 0;
    }
    return -1; 
}


int defer(struct thread_pool *pool, runnable_t runnable) {
    int res = -1;
    if (pool != NULL && !signal_fired) {
        pthread_mutex_lock(&(pool->thread_mutex));
        if (pool->running) {
            push_task(pool, runnable);
            pthread_cond_signal(&(pool->task_cond));
            res = 0;
        }
        pthread_mutex_unlock(&(pool->thread_mutex));
    }
    return res;
}
