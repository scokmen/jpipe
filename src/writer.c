#include <jp_common.h>
#include <jp_file.h>
#include <jp_memory.h>
#include <jp_writer.h>
#include <stdlib.h>
#include <string.h>
#include <sys/uio.h>
#include <unistd.h>

#define IOV_VEC_COUNT 3

JP_ATTR_WEAK
jp_errno_t jp_writer_produce(jp_writer_ctx_t ctx) {
    jp_errno_t err = 0;
    jp_block_t *block, overflow = {0};
    const size_t buf_len = ctx.chunk_size * ctx.encoder.escaping_mul;
    struct iovec iov[IOV_VEC_COUNT];
    jp_file_handler_t file_handler = {0};
    unsigned char *start_ptr, *end_ptr, *delimiter;
    unsigned char* encoded_value = jp_mem_malloc(buf_len);
    overflow.data                = jp_mem_malloc(ctx.chunk_size);
    iov[0].iov_base              = ctx.encoder.prefix_encoder(ctx.fields, &iov[0].iov_len);
    iov[2].iov_base              = ctx.encoder.postfix_encoder(ctx.fields, &iov[2].iov_len);
    iov[1].iov_base              = encoded_value;

    while (true) {
        err = jp_queue_pop_uncommitted(ctx.queue, &block);
        if (err) {
            break;
        }
        start_ptr = block->data;
        end_ptr   = start_ptr + block->length;
        while (start_ptr < end_ptr) {
            delimiter = memchr(start_ptr, '\n', (size_t) (end_ptr - start_ptr));

            if (overflow.length > 0) {
                const size_t remaining_len =
                    delimiter != NULL ? (size_t) (delimiter - start_ptr) : (size_t) (end_ptr - start_ptr);

                const size_t copy_len = MIN(remaining_len, ctx.chunk_size - overflow.length);
                memcpy(overflow.data + overflow.length, start_ptr, copy_len);
                overflow.length += copy_len;
                start_ptr       += copy_len;

                if (delimiter != NULL || overflow.length == ctx.chunk_size) {
                    iov[1].iov_len = ctx.encoder.value_encoder(overflow.data, overflow.length, encoded_value, buf_len);
                    err            = jp_file_writev(&file_handler, iov, IOV_VEC_COUNT);
                    if (err) {
                        goto clean_up;
                    }
                    overflow.length = 0;
                    if (delimiter != NULL && start_ptr == delimiter) {
                        start_ptr++;
                    }
                }
                continue;
            }

            if (delimiter == NULL) {
                size_t remaining = (size_t) (end_ptr - start_ptr);
                memcpy(overflow.data, start_ptr, remaining);
                overflow.length = remaining;
                start_ptr       = end_ptr;
                continue;
            }

            iov[1].iov_len =
                ctx.encoder.value_encoder(start_ptr, (size_t) (delimiter - start_ptr), encoded_value, buf_len);

            err = jp_file_writev(&file_handler, iov, IOV_VEC_COUNT);
            if (err) {
                goto clean_up;
            }
            start_ptr = delimiter + 1;
        }
        jp_queue_pop_commit(ctx.queue);
    }

clean_up:
    if (JP_QUEUE_IS_GRACEFUL_ERR(err)) {
        err = 0;
    }
    for (int i = 0; i < IOV_VEC_COUNT; i++) {
        JP_FREE(iov[i].iov_base);
    }
    JP_FREE(overflow.data);
    jp_queue_finalize(ctx.queue);
    return err;
}