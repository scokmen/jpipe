#ifndef JPIPE_JP_QUEUE_H
#define JPIPE_JP_QUEUE_H

#include <stddef.h>
#include <pthread.h>
#include <jp_common.h>

typedef struct {
    unsigned char *data;
    size_t length;
} jp_block_t;

typedef struct {
    size_t head;
    size_t tail;
    size_t length;
    size_t capacity;
    size_t chunk_size;
    pthread_mutex_t lock;
    pthread_cond_t not_empty;
    pthread_cond_t not_full;
    jp_block_t *blocks;
    unsigned char *area;
} jp_queue_t;

JP_MALLOC
jp_queue_t *jp_queue_create(size_t capacity, size_t chunk_size);

JP_NONNULL_ARG(1, 2)
int jp_queue_push(jp_queue_t *q, const void *src, size_t len);

JP_NONNULL_ARG(1, 2, 3)
int jp_queue_pop(jp_queue_t *q, unsigned char *dest_buffer, size_t *out_len);


void jp_queue_destroy(jp_queue_t *q);

#endif //JPIPE_JP_QUEUE_H
