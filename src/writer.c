#include <jp_common.h>
#include <jp_writer.h>
#include <stdlib.h>
#include <string.h>
#include <sys/uio.h>
#include <unistd.h>

JP_ATTR_WEAK
jp_errno_t jp_writer_produce(jp_writer_ctx_t ctx) {
    jp_errno_t err = 0;
    jp_block_t* block;
    size_t prefix_len, postfix_len, block_len;
    size_t buf_len = ctx.chunk_size * ctx.encoder.escaping_mul;
    struct iovec iov[3];
    unsigned char *start_ptr, *end_ptr, *newline_ptr;
    unsigned char* encoded_value         = malloc(buf_len);
    unsigned const char* encoded_prefix  = ctx.encoder.prefix_encoder(ctx.fields, &prefix_len);
    unsigned const char* encoded_postfix = ctx.encoder.postfix_encoder(ctx.fields, &postfix_len);

    if (encoded_value == NULL || encoded_prefix == NULL || encoded_postfix == NULL) {
        err = JP_ENOMEMORY;
        goto clean_up;
    }

    iov[0].iov_base = (void*) encoded_prefix;
    iov[0].iov_len  = prefix_len;
    iov[1].iov_base = encoded_value;
    iov[2].iov_base = (void*) encoded_postfix;
    iov[2].iov_len  = postfix_len;

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

            const size_t esc_len =
                ctx.encoder.value_encoder(start_ptr, (size_t) (newline_ptr - start_ptr), encoded_value, buf_len);

            iov[1].iov_len = esc_len;
            writev(STDOUT_FILENO, iov, 3);
            start_ptr = newline_ptr + 1;
        }
        jp_queue_pop_commit(ctx.queue);
    }

clean_up:
    if (JP_QUEUE_IS_GRACEFUL_ERR(err)) {
        err = 0;
    } else {
        jp_errno_log_err(err);
    }
    JP_FREE(encoded_value);
    JP_FREE(encoded_prefix);
    JP_FREE(encoded_postfix);
    jp_queue_finalize(ctx.queue);
    return err;
}