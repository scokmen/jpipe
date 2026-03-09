#include <jp_queue.h>
#include <jp_test.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define ITEM_SIZE 200000

typedef void* (*thread_handler)(void*);

void* sequential_write(void* arg) {
    jp_errno_t result = 0;
    int* err          = calloc(1, sizeof(int));
    jp_queue_t* q     = (jp_queue_t*) arg;
    for (int i = 0; i <= ITEM_SIZE; i++) {
        result = jp_queue_push(q, &i, sizeof(int));
        if (result && err != NULL) {
            *err = (int) result;
            pthread_exit(err);
        }
    }
    pthread_exit(err);
}

void* sequential_write_interrupted(void* arg) {
    jp_errno_t result = 0;
    int* err          = calloc(1, sizeof(int));
    jp_queue_t* q     = (jp_queue_t*) arg;
    for (int i = 0; i <= ITEM_SIZE; i++) {
        result = jp_queue_push(q, &i, sizeof(int));
        if (result && err != NULL) {
            *err = (int) result;
            pthread_exit(err);
        }
    }
    jp_queue_finalize(q);
    pthread_exit(err);
}

void* sequential_read(void* arg) {
    int val;
    jp_errno_t result = 0;
    size_t len;
    int64_t sum   = 0;
    int* err      = calloc(1, sizeof(int));
    jp_queue_t* q = (jp_queue_t*) arg;

    for (int i = 0; i <= ITEM_SIZE; i++) {
        result = jp_queue_pop(q, (unsigned char*) &val, sizeof(int), &len);
        if (result && err != NULL) {
            *err = (int) result;
            pthread_exit(err);
        }
        sum += val;
    }

    int64_t expected = (int64_t) ITEM_SIZE * (ITEM_SIZE + 1) / 2;
    if (expected != sum && err != NULL) {
        *err = 1;
    }
    pthread_exit(err);
}

void jp_queue_run_with_args(thread_handler producer, thread_handler consumer) {
    void *producer_result, *consumer_result;
    pthread_t prod_tid, cons_tid;
    jp_queue_t* q = jp_queue_create(16, sizeof(int), JP_QUEUE_POLICY_WAIT);

    pthread_create(&prod_tid, NULL, producer, q);
    pthread_create(&cons_tid, NULL, consumer, q);

    pthread_join(prod_tid, &producer_result);
    JP_ASSERT_NONNULL(producer_result);
    JP_ASSERT_OK(*((int*) producer_result));
    free(producer_result);

    pthread_join(cons_tid, &consumer_result);
    JP_ASSERT_NONNULL(consumer_result);
    JP_ASSERT_OK(*((int*) consumer_result));
    free(consumer_result);

    jp_queue_destroy(q);
}

int main(void) {
    jp_queue_run_with_args(sequential_write, sequential_read);
    jp_queue_run_with_args(sequential_write_interrupted, sequential_read);
    return 0;
}
