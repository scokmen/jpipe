#include <jp_errno.h>
#include <jp_queue.h>
#include <jp_reader.h>
#include <jp_test.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/fcntl.h>
#include <unistd.h>

static void* thread_wrapper(void* arg) {
    const jp_reader_ctx_t* ctx = arg;
    const jp_errno_t err       = jp_reader_consume(*ctx);
    return (void*) (uintptr_t) err;  // NOLINT(performance-no-int-to-ptr)
}

static void test_jp_reader_stream(void) {
    int fds[2];
    jp_errno_t err = 0;
    pthread_t thread;
    jp_block_t* block;
    void* thread_result;
    const char* data  = "stream data\n";
    jp_queue_t* queue = jp_queue_create(16, BYTES_IN_KB, JP_QUEUE_POLICY_WAIT, &err);

    JP_ASSERT_OK(err);
    JP_ASSERT_OK(pipe(fds));

    fcntl(fds[0], F_SETFL, O_NONBLOCK);
    jp_reader_ctx_t ctx = {.input_stream = fds[0], .queue = queue, .chunk_size = BYTES_IN_KB};

    pthread_create(&thread, NULL, thread_wrapper, &ctx);

    const ssize_t n = write(fds[1], data, strlen(data));
    JP_ASSERT_EQ(true, n >= 0);

    close(fds[1]);
    pthread_join(thread, &thread_result);
    JP_ASSERT_OK((int) (uintptr_t) thread_result);
    JP_ASSERT_OK(jp_queue_pop_uncommitted(queue, &block));
    JP_ASSERT_EQ(block->length, strlen(data));
    JP_ASSERT_OK(memcmp(block->data, data, block->length));
    jp_queue_pop_commit(queue);

    jp_queue_destroy(queue);
}

static void test_jp_reader_stream_drop(void) {
    int fds[2];
    jp_errno_t err        = 0;
    const size_t capacity = 4;
    pthread_t thread;
    jp_block_t* block;
    void* thread_result;
    size_t bytes_received = 0;
    const char* data      = "stream data\n";
    jp_queue_t* queue     = jp_queue_create(capacity, BYTES_IN_KB, JP_QUEUE_POLICY_DROP, &err);

    JP_ASSERT_OK(err);
    JP_ASSERT_OK(pipe(fds));

    fcntl(fds[0], F_SETFL, O_NONBLOCK);
    jp_reader_ctx_t ctx = {.input_stream = fds[0], .queue = queue, .chunk_size = BYTES_IN_KB};

    pthread_create(&thread, NULL, thread_wrapper, &ctx);

    for (size_t i = 0; i < capacity * 4; i++) {
        const ssize_t n = write(fds[1], data, strlen(data));
        JP_ASSERT_EQ(true, n >= 0);
        usleep(1000);
    }

    close(fds[1]);
    pthread_join(thread, &thread_result);
    JP_ASSERT_OK((int) (uintptr_t) thread_result);

    while (jp_queue_pop_uncommitted(queue, &block) == 0) {
        bytes_received += block->length;
        jp_queue_pop_commit(queue);
    }
    JP_ASSERT_EQ(strlen(data) * capacity, bytes_received);
    jp_queue_destroy(queue);
}

int main(void) {
    test_jp_reader_stream();
    test_jp_reader_stream_drop();
    return EXIT_SUCCESS;
}
