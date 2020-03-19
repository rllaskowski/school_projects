#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <memory.h>
#include <stdlib.h>
#include "threadpool.h"

typedef struct matrix {
    int *val;
    int *tim;
    int *res;
    int num_col;
    int num_row;
    pthread_mutex_t *row_mutex;
} matrix;

typedef struct task_arg {
    matrix *mat;
    int field;
} task_arg;

void work(void *arg, size_t argsz __attribute__((unused))) {
    task_arg *task = (task_arg*)arg;

    int field = task->field;
    matrix *mat = task->mat;
    int row = field/(mat->num_col);

    usleep(mat->tim[field]);
    
    pthread_mutex_lock(&(mat->row_mutex[row]));
    mat->res[row] += mat->val[field];
    pthread_mutex_unlock(&(mat->row_mutex[row]));
}

int main() {
    thread_pool_t pool;
    matrix mat;
    thread_pool_init(&pool, 4);

    scanf("%d %d", &mat.num_row, &mat.num_col);

    mat.val = malloc(sizeof(*mat.val)*mat.num_col*mat.num_row);
    mat.tim = malloc(sizeof(*mat.tim)*mat.num_col*mat.num_row);
    mat.res = malloc(sizeof(*mat.res)*mat.num_row);
    mat.row_mutex = malloc(sizeof(*mat.row_mutex)*mat.num_row);

    for (int i = 0; i < mat.num_row; i++) {
        pthread_mutex_init(&mat.row_mutex[i], NULL);
        mat.res[i] = 0;
    }

    task_arg *task_args = malloc(sizeof(*task_args)*mat.num_col*mat.num_row);

    for (int i = 0; i < mat.num_col*mat.num_row; i++) {
        scanf("%d %d", &mat.val[i], &mat.tim[i]);
        task_args[i].field = i;
        task_args[i].mat = &mat;
        
        defer(&pool, (runnable_t){
            .function = work,
            .arg = &task_args[i],
            .argsz = sizeof(task_args[i])
        });
    }
    
    thread_pool_destroy(&pool);

    for (int i = 0; i < mat.num_row; i++) {
        printf("%d\n", mat.res[i]);
        pthread_mutex_destroy(&mat.row_mutex[i]);
    }
    
    free(task_args);
    free(mat.val);
    free(mat.tim);
    free(mat.res);
    free(mat.row_mutex);
    return 0;
}