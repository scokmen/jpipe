#include <jp_queue.h>
#include <jp_test.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct {
    int count;
    jp_queue_t* queue;
} test_ctx_t;

typedef void* (*thread_handler)(void*);

void* sequential_write(void* arg) {
    jp_errno_t err    = 0;
    test_ctx_t* ctx   = arg;
    jp_queue_t* queue = ctx->queue;
    jp_block_t* block;

    for (int i = 0; i <= ctx->count; i++) {
        err = jp_queue_push_uncommitted(queue, &block);
        if (err) {
            break;
        }
        *(int*) block->data = i;
        block->length       = sizeof(int);
        jp_queue_push_commit(queue);
    }

    jp_queue_finalize(queue);
    pthread_exit((void*) (uintptr_t) err);  // NOLINT(performance-no-int-to-ptr)
}

void* sequential_read(void* arg) {
    int64_t sum       = 0;
    test_ctx_t* ctx   = arg;
    jp_queue_t* queue = ctx->queue;
    jp_block_t* block;

    while (jp_queue_pop_uncommitted(queue, &block) == 0) {
        sum += *(int*) block->data;
        jp_queue_pop_commit(queue);
    }

    pthread_exit((void*) (uintptr_t) sum);  // NOLINT(performance-no-int-to-ptr)
}

void jp_queue_run_with_args(thread_handler producer, thread_handler consumer, size_t capacity, int count) {
    void *producer_result = NULL, *consumer_result = NULL;
    pthread_t prod_tid, cons_tid;
    jp_queue_t* queue = jp_queue_create(capacity, sizeof(int), JP_QUEUE_POLICY_WAIT);
    test_ctx_t ctx    = {.count = count, .queue = queue};
    int64_t expected  = (int64_t) count * (count + 1) / 2;

    pthread_create(&prod_tid, NULL, producer, &ctx);
    pthread_create(&cons_tid, NULL, consumer, &ctx);

    pthread_join(prod_tid, &producer_result);
    JP_ASSERT_OK((int) (uintptr_t) producer_result);

    pthread_join(cons_tid, &consumer_result);
    JP_ASSERT_EQ((int64_t) (uintptr_t) consumer_result, expected);

    jp_queue_destroy(queue);
}

int main(void) {
    jp_queue_run_with_args(sequential_write, sequential_read, 1, 1000);
    jp_queue_run_with_args(sequential_write, sequential_read, 2, 2000);
    jp_queue_run_with_args(sequential_write, sequential_read, 8, 8000);
    jp_queue_run_with_args(sequential_write, sequential_read, 16, 16000);
    return 0;
}
