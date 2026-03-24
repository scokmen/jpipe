#include <jp_common.h>
#include <jp_queue.h>
#include <stdbool.h>
#include <stdlib.h>

jp_queue_t* jp_queue_create(size_t capacity, size_t chunk_size, jp_queue_policy_t policy) {
    jp_queue_t* queue;
    const size_t blocks_offset = sizeof(jp_queue_t);
    const size_t area_offset   = blocks_offset + capacity * sizeof(jp_block_t);
    const size_t total_size    = area_offset + capacity * chunk_size;

    JP_ALLOC(queue, malloc(total_size), NULL);

    queue->capacity   = capacity;
    queue->chunk_size = chunk_size;
    queue->policy     = policy;
    queue->head       = 0;
    queue->tail       = 0;
    queue->blocks     = (jp_block_t*) ((unsigned char*) queue + blocks_offset);
    queue->area       = (unsigned char*) queue + area_offset;
    atomic_store_explicit(&queue->active, true, memory_order_relaxed);
    atomic_store_explicit(&queue->length, 0, memory_order_relaxed);

    for (size_t i = 0; i < capacity; i++) {
        queue->blocks[i].data   = queue->area + i * chunk_size;
        queue->blocks[i].length = 0;
    }

    pthread_mutex_init(&queue->lock, NULL);
    pthread_cond_init(&queue->not_empty, NULL);
    pthread_cond_init(&queue->not_full, NULL);

    return queue;
}

jp_errno_t jp_queue_push_uncommitted(jp_queue_t* queue, jp_block_t** block) {
    if (JP_ATTR_UNLIKELY(!atomic_load_explicit(&queue->active, memory_order_acquire))) {
        return JP_ESHUTTING_DOWN;
    }

    if (atomic_load_explicit(&queue->length, memory_order_acquire) < queue->capacity) {
        *block = &queue->blocks[queue->tail];
        return 0;
    }

    pthread_mutex_lock(&queue->lock);
    while (queue->policy == JP_QUEUE_POLICY_WAIT && atomic_load_explicit(&queue->active, memory_order_relaxed) &&
           atomic_load_explicit(&queue->length, memory_order_relaxed) >= queue->capacity) {
        pthread_cond_wait(&queue->not_full, &queue->lock);
    }

    if (!atomic_load_explicit(&queue->active, memory_order_relaxed)) {
        pthread_mutex_unlock(&queue->lock);
        return JP_ESHUTTING_DOWN;
    }

    if (queue->policy == JP_QUEUE_POLICY_DROP &&
        atomic_load_explicit(&queue->length, memory_order_relaxed) >= queue->capacity) {
        pthread_mutex_unlock(&queue->lock);
        return JP_EMSG_SHOULD_DROP;
    }

    *block = &queue->blocks[queue->tail];
    pthread_mutex_unlock(&queue->lock);
    return 0;
}

void jp_queue_push_commit(jp_queue_t* queue) {
    pthread_mutex_lock(&queue->lock);
    queue->tail = (queue->tail + 1) % queue->capacity;
    atomic_fetch_add_explicit(&queue->length, 1, memory_order_release);
    pthread_cond_signal(&queue->not_empty);
    pthread_mutex_unlock(&queue->lock);
}

jp_errno_t jp_queue_pop_uncommitted(jp_queue_t* queue, jp_block_t** block) {
    if (atomic_load_explicit(&queue->length, memory_order_acquire) > 0) {
        *block = &queue->blocks[queue->head];
        return 0;
    }

    pthread_mutex_lock(&queue->lock);
    while (atomic_load_explicit(&queue->length, memory_order_relaxed) == 0 &&
           atomic_load_explicit(&queue->active, memory_order_relaxed)) {
        pthread_cond_wait(&queue->not_empty, &queue->lock);
    }

    if (atomic_load_explicit(&queue->length, memory_order_relaxed) == 0 &&
        !atomic_load_explicit(&queue->active, memory_order_relaxed)) {
        pthread_mutex_unlock(&queue->lock);
        return JP_ESHUTTING_DOWN;
    }

    *block = &queue->blocks[queue->head];
    pthread_mutex_unlock(&queue->lock);
    return 0;
}

void jp_queue_pop_commit(jp_queue_t* queue) {
    pthread_mutex_lock(&queue->lock);
    queue->head = (queue->head + 1) % queue->capacity;
    atomic_fetch_sub_explicit(&queue->length, 1, memory_order_release);
    pthread_cond_signal(&queue->not_full);
    pthread_mutex_unlock(&queue->lock);
}

void jp_queue_finalize(jp_queue_t* queue) {
    if (!atomic_load_explicit(&queue->active, memory_order_acquire)) {
        JP_LOG_DEBUG("[QUEUE]: Queue was already finalized.");
        return;
    }
    JP_LOG_DEBUG("[QUEUE]: Queue is being finalized.");
    pthread_mutex_lock(&queue->lock);
    atomic_store_explicit(&queue->active, false, memory_order_release);
    pthread_cond_broadcast(&queue->not_empty);
    pthread_cond_broadcast(&queue->not_full);
    pthread_mutex_unlock(&queue->lock);
}

void jp_queue_destroy(jp_queue_t* queue) {
    if (queue == NULL) {
        return;
    }
    pthread_mutex_destroy(&queue->lock);
    pthread_cond_destroy(&queue->not_empty);
    pthread_cond_destroy(&queue->not_full);
    JP_FREE(queue);
}
