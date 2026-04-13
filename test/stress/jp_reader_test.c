#include <getopt.h>
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

typedef struct {
    int count;
    jp_queue_t* queue;
} test_ctx_t;

static void* reader_thread_wrapper(void* arg) {
    const jp_reader_ctx_t* ctx = arg;
    const jp_errno_t err       = jp_reader_consume(*ctx);
    return (void*) (uintptr_t) err;  // NOLINT(performance-no-int-to-ptr)
}

static void* checker_thread_wrapper(void* arg) {
    jp_block_t* block;
    size_t bytes_received = 0;
    const test_ctx_t* ctx = arg;
    while (jp_queue_pop_uncommitted(ctx->queue, &block) == 0) {
        bytes_received += block->length;
        jp_queue_pop_commit(ctx->queue);
    }
    return (void*) bytes_received;  // NOLINT(performance-no-int-to-ptr)
}

static void test_jp_reader_stream_with_args(size_t capacity, size_t chunk_size, int count) {
    int fds[2];
    jp_errno_t err = 0;
    pthread_t reader_thread, checker_thread;
    void *reader_result, *checker_result;
    const char* data  = "stream data\n";
    jp_queue_t* queue = jp_queue_create(capacity, chunk_size, JP_QUEUE_POLICY_WAIT, &err);

    JP_ASSERT_OK(err);
    JP_ASSERT_OK(pipe(fds));

    fcntl(fds[0], F_SETFL, O_NONBLOCK);
    test_ctx_t test_ctx        = {.queue = queue, .count = count};
    jp_reader_ctx_t reader_ctx = {.input_stream = fds[0], .queue = queue, .chunk_size = chunk_size};

    pthread_create(&reader_thread, NULL, reader_thread_wrapper, &reader_ctx);
    pthread_create(&checker_thread, NULL, checker_thread_wrapper, &test_ctx);

    for (int i = 0; i < count; i++) {
        const ssize_t n = write(fds[1], data, strlen(data));
        JP_ASSERT_EQ(true, n >= 0);
    }

    close(fds[1]);
    pthread_join(reader_thread, &reader_result);
    pthread_join(checker_thread, &checker_result);

    JP_ASSERT_OK((int) (uintptr_t) reader_result);
    JP_ASSERT_EQ((strlen(data) * (size_t) count), (size_t) (checker_result));

    jp_queue_destroy(queue);
}

int main(void) {
    test_jp_reader_stream_with_args(1, 4, 1000);
    test_jp_reader_stream_with_args(2, 4, 2000);
    test_jp_reader_stream_with_args(8, 4, 8000);
    test_jp_reader_stream_with_args(16, 4, 16000);
    test_jp_reader_stream_with_args(1, BYTES_IN_KB, 1000);
    test_jp_reader_stream_with_args(2, BYTES_IN_KB, 2000);
    test_jp_reader_stream_with_args(8, BYTES_IN_KB, 8000);
    test_jp_reader_stream_with_args(16, BYTES_IN_KB, 16000);
    return EXIT_SUCCESS;
}
