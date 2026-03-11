#ifndef JPIPE_JP_QUEUE_H
#define JPIPE_JP_QUEUE_H

#include <jp_common.h>
#include <jp_errno.h>
#include <pthread.h>
#include <stdbool.h>

typedef enum {
    JP_QUEUE_POLICY_WAIT = 0,
    JP_QUEUE_POLICY_DROP = 1
} jp_queue_policy_t;

typedef struct {
    unsigned char* data JP_NONSTRING;
    size_t length;
} jp_block_t;

typedef struct {
    bool active;
    size_t head;
    size_t tail;
    size_t length;
    size_t capacity;
    size_t chunk_size;
    pthread_mutex_t lock;
    pthread_cond_t not_empty;
    pthread_cond_t not_full;
    jp_queue_policy_t policy;
    jp_block_t* blocks;
    unsigned char* area JP_NONSTRING;
} jp_queue_t;

JP_MALLOC
JP_USE_RESULT
jp_queue_t* jp_queue_create(size_t capacity, size_t chunk_size, jp_queue_policy_t policy);

JP_NONNULL_ARG(1, 2)
JP_USE_RESULT
jp_errno_t jp_queue_push_uncommitted(jp_queue_t* queue, jp_block_t** block);

JP_NONNULL_ARG(1)
void jp_queue_push_commit(jp_queue_t* queue);

JP_NONNULL_ARG(1, 2)
jp_errno_t jp_queue_pop_uncommitted(jp_queue_t* queue, jp_block_t** block);

JP_NONNULL_ARG(1)
void jp_queue_pop_commit(jp_queue_t* queue);

JP_NONNULL_ARG(1)
void jp_queue_finalize(jp_queue_t* queue);

void jp_queue_destroy(jp_queue_t* queue);

#endif  // JPIPE_JP_QUEUE_H
