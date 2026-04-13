#include <jp_errno.h>
#include <jp_file.h>
#include <jp_memory.h>
#include <jp_queue.h>
#include <jp_test.h>
#include <jp_writer.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/fcntl.h>
#include <unistd.h>

typedef struct {
    int item_count;
    jp_writer_ctx_t* ctx;
    const char* data;
    size_t data_len;
} test_case_t;

static size_t total_bytes_written = 0;

jp_errno_t jp_file_writev(JP_ATTR_UNUSED jp_file_handler_t* handler, struct iovec* iov, int count) {
    for (int i = 0; i < count; i++) {
        total_bytes_written += iov[i].iov_len;
    }
    return 0;
}

static unsigned char* mock_prefix_postfix_encoder(JP_ATTR_UNUSED const jp_field_set_t* field_set, size_t* out_len) {
    *out_len = 1;
    return jp_mem_strdup("x");
}

static size_t mock_value_encoder(const unsigned char* restrict src,
                                 size_t src_len,
                                 unsigned char* restrict dst,
                                 JP_ATTR_UNUSED size_t dst_len) {
    memcpy(dst, src, src_len);
    return src_len;
}

static jp_encoder_t encoder = {.prefix_encoder  = mock_prefix_postfix_encoder,
                               .value_encoder   = mock_value_encoder,
                               .postfix_encoder = mock_prefix_postfix_encoder,
                               .escaping_mul    = 1};

static void* data_initializer(void* arg) {
    const test_case_t* test_case = arg;
    jp_queue_t* queue            = test_case->ctx->queue;
    jp_block_t* block            = NULL;
    for (int i = 0; i < test_case->item_count; i++) {
        if (jp_queue_push_uncommitted(queue, &block) == 0) {
            memcpy(block->data, test_case->data, test_case->data_len);
            block->length = test_case->data_len;
            jp_queue_push_commit(queue);
        }
    }
    jp_queue_finalize(queue);
    return NULL;
}

static void* writer_wrapper(void* arg) {
    const test_case_t* test_case = arg;
    const jp_errno_t err         = jp_writer_produce(*test_case->ctx);
    return (void*) (uintptr_t) err;  // NOLINT(performance-no-int-to-ptr)
}

static void test_jp_writer_consume_with_args(int item_count, size_t chunk_size) {
    total_bytes_written   = 0;
    jp_errno_t err        = 0;
    const size_t capacity = 1024;
    jp_queue_t* queue     = jp_queue_create(capacity, chunk_size, JP_QUEUE_POLICY_WAIT, &err);
    pthread_t init_thread, writer_thread;
    void* writer_result;

    JP_ASSERT_OK(err);
    JP_ASSERT_NONNULL(queue);

    jp_writer_ctx_t ctx = {
        .chunk_size = chunk_size,
        .queue      = queue,
        .output_dir = ".",
        .fields     = NULL,
        .encoder    = encoder,
    };

    test_case_t test_case = {.item_count = item_count, .ctx = &ctx, .data = "data\n", .data_len = 5};

    pthread_create(&init_thread, NULL, data_initializer, &test_case);
    pthread_create(&writer_thread, NULL, writer_wrapper, &test_case);

    pthread_join(init_thread, NULL);
    pthread_join(writer_thread, &writer_result);

    JP_ASSERT_OK((int) (uintptr_t) writer_result);
    JP_ASSERT_EQ((size_t) (6 * item_count), total_bytes_written);

    jp_queue_destroy(queue);
}

int main(void) {
    test_jp_writer_consume_with_args(1, 16);
    test_jp_writer_consume_with_args(10, 16);
    test_jp_writer_consume_with_args(100, 16);
    test_jp_writer_consume_with_args(1000, 16);
    test_jp_writer_consume_with_args(10000, 16);
    test_jp_writer_consume_with_args(1, BYTES_IN_KB);
    test_jp_writer_consume_with_args(10, BYTES_IN_KB);
    test_jp_writer_consume_with_args(100, BYTES_IN_KB);
    test_jp_writer_consume_with_args(1000, BYTES_IN_KB);
    test_jp_writer_consume_with_args(10000, BYTES_IN_KB);
    return EXIT_SUCCESS;
}
