#include <jp_queue.h>
#include <jp_test.h>
#include <stdio.h>
#include <stdlib.h>

#define ITEM_SIZE 4

void test_jp_queue_push_pop(void) {
    int data[ITEM_SIZE] = {1, 2, 3, 4};
    jp_queue_t* q       = jp_queue_create(4, sizeof(int), JP_QUEUE_POLICY_WAIT);
    jp_block_t *w_block, *r_block;

    JP_ASSERT_EQ(ITEM_SIZE, q->capacity);
    for (size_t i = 0; i < ITEM_SIZE; i++) {
        JP_ASSERT_EQ(i, q->length);
        JP_ASSERT_OK(jp_queue_push_uncommitted(q, &w_block));
        *(int*) w_block->data = data[i];
        w_block->length       = sizeof(int);
        jp_queue_push_commit(q);
        JP_ASSERT_EQ(i + 1, q->length);
    }

    for (size_t i = 0; i < ITEM_SIZE; i++) {
        JP_ASSERT_EQ(ITEM_SIZE - i, q->length);
        JP_ASSERT_OK(jp_queue_pop_uncommitted(q, &r_block));
        jp_queue_pop_commit(q);
        JP_ASSERT_EQ(ITEM_SIZE - i - 1, q->length);
        JP_ASSERT_EQ(*(int*) r_block->data, data[i]);
    }

    jp_queue_destroy(q);
}

void test_jp_queue_push_finalized_queue(void) {
    int data[ITEM_SIZE] = {1, 2, 3, 4};
    jp_queue_t* q       = jp_queue_create(4, sizeof(int), JP_QUEUE_POLICY_WAIT);
    jp_block_t* w_block;

    JP_ASSERT_EQ(ITEM_SIZE, q->capacity);
    for (size_t i = 0; i < 2; i++) {
        JP_ASSERT_OK(jp_queue_push_uncommitted(q, &w_block));
        *(int*) w_block->data = data[i];
        w_block->length       = sizeof(int);
        jp_queue_push_commit(q);
        JP_ASSERT_EQ(i + 1, q->length);
    }

    jp_queue_finalize(q);
    JP_ASSERT_EQ(JP_ESHUTTING_DOWN, jp_queue_push_uncommitted(q, &w_block));
    jp_queue_destroy(q);
}

void test_jp_queue_pop_finalized_queue(void) {
    int data[ITEM_SIZE] = {1, 2, 3, 4};
    jp_queue_t* q       = jp_queue_create(4, sizeof(int), JP_QUEUE_POLICY_WAIT);
    jp_block_t *w_block, *r_block;
    JP_ASSERT_EQ(ITEM_SIZE, q->capacity);
    for (size_t i = 0; i < ITEM_SIZE; i++) {
        JP_ASSERT_OK(jp_queue_push_uncommitted(q, &w_block));
        *(int*) w_block->data = data[i];
        w_block->length       = sizeof(int);
        jp_queue_push_commit(q);
        JP_ASSERT_EQ(i + 1, q->length);
    }

    jp_queue_finalize(q);
    for (size_t i = 0; i < ITEM_SIZE; i++) {
        JP_ASSERT_OK(jp_queue_pop_uncommitted(q, &r_block));
        jp_queue_pop_commit(q);
    }

    JP_ASSERT_EQ(JP_ESHUTTING_DOWN, jp_queue_pop_uncommitted(q, &r_block));
    jp_queue_destroy(q);
}

void test_jp_queue_pop_policy_drop(void) {
    int data[ITEM_SIZE] = {1, 2, 3, 4};
    jp_queue_t* q       = jp_queue_create(2, sizeof(int), JP_QUEUE_POLICY_DROP);
    jp_block_t *w_block, *r_block;

    JP_ASSERT_EQ(2, q->capacity);
    for (size_t i = 0; i < 2; i++) {
        JP_ASSERT_EQ(i, q->length);
        JP_ASSERT_OK(jp_queue_push_uncommitted(q, &w_block));
        *(int*) w_block->data = data[i];
        w_block->length       = sizeof(int);
        jp_queue_push_commit(q);
        JP_ASSERT_EQ(i + 1, q->length);
    }

    for (size_t i = 0; i < 2; i++) {
        JP_ASSERT_EQ(2, q->length);
        JP_ASSERT_EQ(JP_EMSG_SHOULD_DROP, jp_queue_push_uncommitted(q, &w_block));
        JP_ASSERT_EQ(2, q->length);
    }

    for (size_t i = 0; i < 2; i++) {
        JP_ASSERT_EQ(2 - i, q->length);
        JP_ASSERT_OK(jp_queue_pop_uncommitted(q, &r_block));
        jp_queue_pop_commit(q);
        JP_ASSERT_EQ(2 - i - 1, q->length);
        JP_ASSERT_EQ(*(int*) r_block->data, data[i]);
    }

    jp_queue_destroy(q);
}

int main(void) {
    test_jp_queue_push_pop();
    test_jp_queue_push_finalized_queue();
    test_jp_queue_pop_finalized_queue();
    test_jp_queue_pop_policy_drop();
    return 0;
}
