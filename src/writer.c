#include <jp_common.h>
#include <jp_memory.h>
#include <jp_writer.h>
#include <stdlib.h>
#include <string.h>
#include <sys/uio.h>
#include <unistd.h>

JP_ATTR_WEAK
jp_errno_t jp_writer_produce(jp_writer_ctx_t ctx) {
    jp_errno_t err = 0;
    jp_block_t* block;
    size_t block_len;
    size_t buf_len = ctx.chunk_size * ctx.encoder.escaping_mul;
    struct iovec iov[3];
    unsigned char *start_ptr, *end_ptr, *newline_ptr;
    unsigned char* encoded_value = jp_mem_malloc(buf_len);
    iov[0].iov_base              = ctx.encoder.prefix_encoder(ctx.fields, &iov[0].iov_len);
    iov[2].iov_base              = ctx.encoder.postfix_encoder(ctx.fields, &iov[2].iov_len);
    iov[1].iov_base              = encoded_value;

    while (true) {
        err = jp_queue_pop_uncommitted(ctx.queue, &block);
        if (err) {
            break;
        }
        block_len = block->length;
        start_ptr = block->data;
        end_ptr   = start_ptr + block_len;
        while (start_ptr < end_ptr) {
            newline_ptr = memchr(start_ptr, '\n', (size_t) (end_ptr - start_ptr));

            if (newline_ptr == NULL) {
                // TODO: Handle overflow.
                break;
            }

            iov[1].iov_len =
                ctx.encoder.value_encoder(start_ptr, (size_t) (newline_ptr - start_ptr), encoded_value, buf_len);

            if (writev(STDOUT_FILENO, iov, 3) < 0) {
                // TODO: Handle error;
            }
            start_ptr = newline_ptr + 1;
        }
        jp_queue_pop_commit(ctx.queue);
    }

    if (JP_QUEUE_IS_GRACEFUL_ERR(err)) {
        err = 0;
    }
    JP_FREE(iov[0].iov_base);
    JP_FREE(iov[1].iov_base);
    JP_FREE(iov[2].iov_base);
    jp_queue_finalize(ctx.queue);
    return err;
}