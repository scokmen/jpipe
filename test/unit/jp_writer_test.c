#include <jp_errno.h>
#include <jp_file.h>
#include <jp_memory.h>
#include <jp_queue.h>
#include <jp_test.h>
#include <jp_writer.h>
#include <stdlib.h>
#include <string.h>
#include <sys/fcntl.h>
#include <unistd.h>

typedef struct {
    int number_of_loop;
    jp_writer_ctx_t* ctx;
    const char* data;
    size_t data_len;
} test_case_t;

static size_t total_bytes_written = 0;
static jp_errno_t write_err       = 0;

jp_errno_t jp_file_writev(JP_ATTR_UNUSED jp_file_handler_t* handler, struct iovec* iov, int count) {
    for (int i = 0; i < count; i++) {
        total_bytes_written += iov[i].iov_len;
    }
    return write_err;
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

static void test_jp_writer_consume(void) {
    write_err               = 0;
    total_bytes_written     = 0;
    jp_errno_t err          = 0;
    jp_block_t* block       = NULL;
    const size_t capacity   = 4;
    const size_t chunk_size = 16;
    jp_queue_t* queue       = jp_queue_create(capacity, chunk_size, JP_QUEUE_POLICY_WAIT, &err);

    JP_ASSERT_OK(err);
    JP_ASSERT_NONNULL(queue);

    jp_writer_ctx_t ctx = {
        .chunk_size = chunk_size,
        .queue      = queue,
        .output_dir = ".",
        .fields     = NULL,
        .encoder    = encoder,
    };

    JP_ASSERT_OK(jp_queue_push_uncommitted(queue, &block));
    memcpy(block->data, "data\n", 5);
    block->length = 5;
    jp_queue_push_commit(queue);
    jp_queue_finalize(queue);

    err = jp_writer_produce(ctx);
    JP_ASSERT_OK(err);
    JP_ASSERT_EQ(6, total_bytes_written);

    jp_queue_destroy(queue);
}

static void test_jp_writer_consume_overflow(void) {
    write_err               = 0;
    total_bytes_written     = 0;
    jp_errno_t err          = 0;
    jp_block_t* block       = NULL;
    const size_t capacity   = 4;
    const size_t chunk_size = BYTES_IN_KB;
    jp_queue_t* queue       = jp_queue_create(capacity, chunk_size, JP_QUEUE_POLICY_WAIT, &err);

    JP_ASSERT_OK(err);
    JP_ASSERT_NONNULL(queue);

    jp_writer_ctx_t ctx = {
        .chunk_size = chunk_size,
        .queue      = queue,
        .output_dir = ".",
        .fields     = NULL,
        .encoder    = encoder,
    };

    JP_ASSERT_OK(jp_queue_push_uncommitted(queue, &block));
    memcpy(block->data, "data", 4);
    block->length = 4;
    jp_queue_push_commit(queue);

    JP_ASSERT_OK(jp_queue_push_uncommitted(queue, &block));
    memcpy(block->data, "da\nta", 5);
    block->length = 5;
    jp_queue_push_commit(queue);

    JP_ASSERT_OK(jp_queue_push_uncommitted(queue, &block));
    memcpy(block->data, "\ndata\n", 6);
    block->length = 6;
    jp_queue_push_commit(queue);

    jp_queue_finalize(queue);

    err = jp_writer_produce(ctx);
    JP_ASSERT_OK(err);
    JP_ASSERT_EQ(18, total_bytes_written);

    jp_queue_destroy(queue);
}

static void test_jp_writer_consume_write_error(void) {
    write_err               = JP_EWRITE_FAILED;
    total_bytes_written     = 0;
    jp_errno_t err          = 0;
    jp_block_t* block       = NULL;
    const size_t capacity   = 4;
    const size_t chunk_size = 16;
    jp_queue_t* queue       = jp_queue_create(capacity, chunk_size, JP_QUEUE_POLICY_WAIT, &err);

    JP_ASSERT_OK(err);
    JP_ASSERT_NONNULL(queue);

    jp_writer_ctx_t ctx = {
        .chunk_size = chunk_size,
        .queue      = queue,
        .output_dir = ".",
        .fields     = NULL,
        .encoder    = encoder,
    };

    JP_ASSERT_OK(jp_queue_push_uncommitted(queue, &block));
    memcpy(block->data, "data\n", 5);
    block->length = 5;
    jp_queue_push_commit(queue);
    jp_queue_finalize(queue);

    err = jp_writer_produce(ctx);
    JP_ASSERT_EQ(JP_EWRITE_FAILED, err);

    jp_queue_destroy(queue);
}

int main(void) {
    test_jp_writer_consume();
    test_jp_writer_consume_overflow();
    test_jp_writer_consume_write_error();
    return EXIT_SUCCESS;
}
