#include <jp_queue.h>
#include <jp_test.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define ITEM_SIZE 20000

typedef void* (*thread_handler)(void*);

void* sequential_write(void* arg) {
    jp_errno_t err = 0;
    jp_queue_t* q  = arg;
    jp_block_t* block;

    for (int i = 0; i <= ITEM_SIZE; i++) {
        err = jp_queue_push_uncommitted(q, &block);
        if (err) {
            break;
        }
        *(int*) block->data = i;
        block->length       = sizeof(int);
        jp_queue_push_commit(q);
    }

    jp_queue_finalize(q);
    pthread_exit((void*) (uintptr_t) err);  // NOLINT(performance-no-int-to-ptr)
}

void* sequential_read(void* arg) {
    int64_t sum   = 0;
    jp_queue_t* q = arg;
    jp_block_t* block;

    while (jp_queue_pop_uncommitted(q, &block) == 0) {
        sum += *(int*) block->data;
        jp_queue_pop_commit(q);
    }

    pthread_exit((void*) (uintptr_t) sum);  // NOLINT(performance-no-int-to-ptr)
}

void jp_queue_run_with_args(thread_handler producer, thread_handler consumer) {
    void *producer_result = NULL, *consumer_result = NULL;
    pthread_t prod_tid, cons_tid;
    jp_queue_t* q    = jp_queue_create(16, sizeof(int), JP_QUEUE_POLICY_WAIT);
    int64_t expected = (int64_t) ITEM_SIZE * (ITEM_SIZE + 1) / 2;

    pthread_create(&prod_tid, NULL, producer, q);
    pthread_create(&cons_tid, NULL, consumer, q);

    pthread_join(prod_tid, &producer_result);
    JP_ASSERT_OK((int) (uintptr_t) producer_result);

    pthread_join(cons_tid, &consumer_result);
    JP_ASSERT_EQ((int64_t) (uintptr_t) consumer_result, expected);

    jp_queue_destroy(q);
}

int main(void) {
    jp_queue_run_with_args(sequential_write, sequential_read);
    return 0;
}
