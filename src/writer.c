#include <jp_common.h>
#include <jp_writer.h>
#include <stdlib.h>

JP_ATTR_WEAK
jp_errno_t jp_writer_produce(jp_writer_ctx_t ctx) {
    jp_errno_t err = 0;
    jp_block_t* block;
    size_t prefix_len, postfix_len;
    unsigned char* buffer        = malloc(ctx.chunk_size);
    unsigned const char* prefix  = ctx.encoder.prefix_encoder(ctx.fields, &prefix_len);
    unsigned const char* postfix = ctx.encoder.postfix_encoder(ctx.fields, &postfix_len);
    if (buffer == NULL || prefix == NULL || postfix == NULL) {
        err = JP_ENOMEMORY;
        goto clean_up;
    }

    while (true) {
        err = jp_queue_pop_uncommitted(ctx.queue, &block);
        if (err) {
            break;
        }
        const size_t esc_len = ctx.encoder.value_encoder(block->data, block->length, buffer, ctx.chunk_size);
        fprintf(stdout,
                "%.*s\"%.*s%.*s",
                (int) prefix_len,
                (char*) prefix,
                (int) esc_len,
                (char*) buffer,
                (int) postfix_len,
                (char*) postfix);
        jp_queue_pop_commit(ctx.queue);
    }

clean_up:
    if (JP_QUEUE_IS_GRACEFUL_ERR(err)) {
        err = 0;
    } else {
        jp_errno_log_err(err);
    }
    JP_FREE(buffer);
    JP_FREE(prefix);
    JP_FREE(postfix);
    jp_queue_finalize(ctx.queue);
    return err;
}