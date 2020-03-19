#include <stdbool.h>

#include "future.h"

typedef void *(*function_t)(void *);


void worker(void *arg, size_t __attribute__((unused))argsz) {
  future_t *task = (future_t*)arg;

  task->res = task->callable.function(task->callable.arg, task->callable.argsz, &(task->res_size));
  task->done = true;
}

int async(thread_pool_t *pool, future_t *future, callable_t callable) {
    if (future != NULL) {
      future->callable = callable;
      future->done = false;
      future->started = true;

      return defer(pool, (runnable_t){
        .function = worker,
        .arg = (void *)future,
        .argsz = sizeof(*future)
      });

      return 0;
    }
    return -1; 
}

int map(thread_pool_t *pool, future_t *future, future_t *from,
        void *(*function)(void *, size_t, size_t *)) {
    
    void *res = await(from);

    if (res == NULL) {
      return -1;
    }

    callable_t callable = (callable_t){ 
      .function = function,
      .arg = res,
      .argsz = from->res_size
    };

    return async(pool, future, callable);
}

void *await(future_t *future) {
    if (future != NULL && future->started) {
      while (!future->done);
      return future->res;
    }
    return NULL;
}
