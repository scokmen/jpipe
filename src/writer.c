#include <jp_writer.h>
#include <stdlib.h>

JP_ATTR_WEAK
jp_errno_t jp_writer_produce(jp_writer_ctx_t ctx) {
    jp_errno_t err = 0;
    jp_block_t* block;
    unsigned char* buffer = malloc(ctx.chunk_size);
    if (buffer == NULL) {
        err = JP_ENOMEMORY;
        goto clean_up;
    }

    while (true) {
        err = jp_queue_pop_uncommitted(ctx.queue, &block);
        if (err) {
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
    JP_FREE(buffer);
    jp_queue_finalize(ctx.queue);
    return err;
}