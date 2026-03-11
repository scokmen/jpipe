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
JP_READ_PTR_SIZE(2, 3)
jp_errno_t jp_queue_push(jp_queue_t* queue, const void* src, size_t len);

JP_NONNULL_ARG(1, 2, 4)
JP_WRITE_PTR_SIZE(2, 3)
jp_errno_t jp_queue_pop(jp_queue_t* queue, unsigned char* dest_buffer, size_t max_len, size_t* out_len);

JP_NONNULL_ARG(1)
void jp_queue_finalize(jp_queue_t* queue);

void jp_queue_destroy(jp_queue_t* queue);

#endif  // JPIPE_JP_QUEUE_H
