#include <pthread.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <jp_queue.h>
#include <jp_test.h>

#define ITEM_SIZE 2000000

void *producer(void *arg) {
    jp_queue_t *q = (jp_queue_t *) arg;

    for (int i = 1; i <= ITEM_SIZE; i++) {
        jp_queue_push(q, &i, sizeof(int));
    }
    return NULL;
}

void *consumer(void *arg) {
    int val;
    size_t len;
    int64_t sum = 0;
    jp_queue_t *q = (jp_queue_t *) arg;

    for (int i = 0; i < ITEM_SIZE; i++) {
        jp_queue_pop(q, (unsigned char *) &val, &len);
        sum += val;
    }

    int64_t expected = (int64_t) ITEM_SIZE * (ITEM_SIZE + 1) / 2;
    JP_ASSERT_OK((int)(expected - sum));
    return NULL;
}

int main() {
    pthread_t prod_tid, cons_tid;
    jp_queue_t *q = jp_queue_create(16, sizeof(int));
    
    pthread_create(&prod_tid, NULL, producer, q);
    pthread_create(&cons_tid, NULL, consumer, q);
    pthread_join(prod_tid, NULL);
    pthread_join(cons_tid, NULL);
    
    jp_queue_destroy(q);
    return 0;
}