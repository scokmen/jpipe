#ifndef JPIPE_JP_QUEUE_H
#define JPIPE_JP_QUEUE_H

#include <jp_common.h>
#include <jp_errno.h>
#include <pthread.h>
#include <stdatomic.h>

/**
 * @brief Identifies non-fatal, expected queue states.
 *
 * These signals indicate a controlled change in flow rather than a failure:
 * - JP_ESHUTTING_DOWN: The system is performing a coordinated stop.
 * - JP_EMSG_SHOULD_DROP: Flow control is active; data is discarded to prevent blocking.
 */
#define JP_QUEUE_IS_GRACEFUL_ERR(err) ((err) == JP_ESHUTTING_DOWN || (err) == JP_EMSG_SHOULD_DROP)

typedef enum {
    JP_QUEUE_POLICY_WAIT = 0,
    JP_QUEUE_POLICY_DROP = 1
} jp_queue_policy_t;

typedef struct {
    unsigned char* data JP_ATTR_BUFFER;
    size_t length;
} jp_block_t;

typedef struct {
    size_t head;
    size_t tail;
    size_t capacity;
    size_t chunk_size;
    atomic_bool active;
    atomic_size_t length;
    pthread_mutex_t lock;
    pthread_cond_t not_empty;
    pthread_cond_t not_full;
    jp_queue_policy_t policy;
    jp_block_t* blocks;
    unsigned char* area JP_ATTR_BUFFER;
} jp_queue_t;

JP_ATTR_ALLOCATED
JP_ATTR_USE_RETURN
jp_queue_t* jp_queue_create(size_t capacity, size_t chunk_size, jp_queue_policy_t policy);

JP_ATTR_NONNULL(1, 2)
jp_errno_t jp_queue_push_uncommitted(jp_queue_t* queue, jp_block_t** block);

JP_ATTR_NONNULL(1)
void jp_queue_push_commit(jp_queue_t* queue);

JP_ATTR_NONNULL(1, 2)
jp_errno_t jp_queue_pop_uncommitted(jp_queue_t* queue, jp_block_t** block);

JP_ATTR_NONNULL(1)
void jp_queue_pop_commit(jp_queue_t* queue);

JP_ATTR_NONNULL(1)
void jp_queue_finalize(jp_queue_t* queue);

void jp_queue_destroy(jp_queue_t* queue);

#endif  // JPIPE_JP_QUEUE_H
