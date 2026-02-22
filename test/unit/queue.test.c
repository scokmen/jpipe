#include <stdio.h>
#include <stdlib.h>
#include <jp_queue.h>
#include <jp_test.h>

#define ITEM_SIZE  4

void test_jp_queue_push_pop(void) {
    int v;
    size_t len;
    int data[ITEM_SIZE] = {1, 2, 3, 4};
    jp_queue_t *q = jp_queue_create(4, sizeof(int));

    JP_ASSERT_EQ(ITEM_SIZE, q->capacity);
    for (int i = 0; i < ITEM_SIZE; i++) {
        JP_ASSERT_EQ(i, q->length);
        JP_ASSERT_OK(jp_queue_push(q, &data[i], sizeof(int)));
        JP_ASSERT_EQ(i + 1, q->length);
    }

    for (int i = 0; i < ITEM_SIZE; i++) {
        JP_ASSERT_EQ(ITEM_SIZE - i, q->length);
        JP_ASSERT_OK(jp_queue_pop(q, (unsigned char *) &v, &len));
        JP_ASSERT_EQ(ITEM_SIZE - i - 1, q->length);
        JP_ASSERT_EQ(v, data[i]);
    }

    jp_queue_destroy(q);
}


int main() {
    test_jp_queue_push_pop();
    return 0;
}