#include "jp_queue.h"
#include <stdlib.h>
#include <string.h>

jp_queue_t *jp_queue_create(size_t capacity, size_t chunk_size) {
    jp_queue_t *queue;

    size_t blocks_offset = sizeof(jp_queue_t);
    size_t area_offset = blocks_offset + (capacity * sizeof(jp_block_t));
    size_t total_size = area_offset + (capacity * chunk_size);

    JP_ALLOC_OR_RET(queue, malloc(total_size), NULL);

    queue->capacity = capacity;
    queue->chunk_size = chunk_size;
    queue->head = 0;
    queue->tail = 0;
    queue->length = 0;
    queue->blocks = (jp_block_t *) ((unsigned char *) queue + blocks_offset);
    queue->area = (unsigned char *) queue + area_offset;

    for (size_t i = 0; i < capacity; i++) {
        queue->blocks[i].data = queue->area + (i * chunk_size);
        queue->blocks[i].length = 0;
    }

    pthread_mutex_init(&queue->lock, NULL);
    pthread_cond_init(&queue->not_empty, NULL);
    pthread_cond_init(&queue->not_full, NULL);

    return queue;
}

int jp_queue_push(jp_queue_t *queue, const void *src, size_t len) {
    pthread_mutex_lock(&queue->lock);

    while (queue->length == queue->capacity) {
        pthread_cond_wait(&queue->not_full, &queue->lock);
    }

    size_t block_size = (len > queue->chunk_size) ? queue->chunk_size : len;
    memcpy(queue->blocks[queue->tail].data, src, block_size);
    queue->blocks[queue->tail].length = block_size;
    queue->tail = (queue->tail + 1) % queue->capacity;
    queue->length++;

    pthread_cond_signal(&queue->not_empty);
    pthread_mutex_unlock(&queue->lock);
    return 0;
}

int jp_queue_pop(jp_queue_t *queue, unsigned char *dest_buffer, size_t *out_len) {
    pthread_mutex_lock(&queue->lock);

    while (queue->length == 0) {
        pthread_cond_wait(&queue->not_empty, &queue->lock);
    }

    size_t block_len = queue->blocks[queue->head].length;
    memcpy(dest_buffer, queue->blocks[queue->head].data, block_len);
    *out_len = block_len;

    queue->head = (queue->head + 1) % queue->capacity;
    queue->length--;

    pthread_cond_signal(&queue->not_full);
    pthread_mutex_unlock(&queue->lock);

    return 0;
}

void jp_queue_destroy(jp_queue_t *queue) {
    if (queue == NULL) {
        return;
    }

    pthread_mutex_destroy(&queue->lock);
    pthread_cond_destroy(&queue->not_empty);
    pthread_cond_destroy(&queue->not_full);
    JP_FREE_IF_ALLOC(queue);
}
