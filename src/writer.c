#include <jp_common.h>
#include <jp_writer.h>
#include <stdlib.h>
#include <string.h>

JP_ATTR_WEAK
jp_errno_t jp_writer_produce(jp_writer_ctx_t ctx) {
    jp_errno_t err = 0;
    jp_block_t* block;
    size_t prefix_len, postfix_len, block_len;
    unsigned char *start_ptr, *end_ptr, *newline_ptr;
    unsigned char* encoded_value         = malloc(ctx.chunk_size * 6);
    unsigned const char* encoded_prefix  = ctx.encoder.prefix_encoder(ctx.fields, &prefix_len);
    unsigned const char* encoded_postfix = ctx.encoder.postfix_encoder(ctx.fields, &postfix_len);

    if (encoded_value == NULL || encoded_prefix == NULL || encoded_postfix == NULL) {
        err = JP_ENOMEMORY;
        goto clean_up;
    }

    while (true) {
        err = jp_queue_pop_uncommitted(ctx.queue, &block);
        if (err) {
            break;
        }
        block_len   = block->length;
        start_ptr   = block->data;
        end_ptr     = start_ptr + block_len;
        newline_ptr = memchr(start_ptr, '\n', block_len);
        if (newline_ptr == NULL) {
            block->data[block_len - 1] = '\n';
            newline_ptr                = end_ptr;
        }
        while ((newline_ptr = memchr(start_ptr, '\n', (size_t) (end_ptr - newline_ptr))) != NULL) {
            const size_t esc_len =
                ctx.encoder.value_encoder(start_ptr, (size_t) (newline_ptr - start_ptr), encoded_value, ctx.chunk_size);

            fprintf(stdout,
                    "%.*s\"%.*s%.*s",
                    (int) prefix_len,
                    (char*) encoded_prefix,
                    (int) esc_len,
                    (char*) encoded_value,
                    (int) postfix_len,
                    (char*) encoded_postfix);

            if (newline_ptr < end_ptr) {
                start_ptr = newline_ptr + 1;
                continue;
            }
            break;
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