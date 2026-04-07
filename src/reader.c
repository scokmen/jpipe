#include <jp_memory.h>
#include <jp_poller.h>
#include <jp_queue.h>
#include <jp_reader.h>
#include <stdlib.h>
#include <unistd.h>

JP_ATTR_WEAK
jp_errno_t jp_reader_consume(jp_reader_ctx_t ctx) {
    jp_errno_t err        = 0;
    ssize_t read_len      = 0;
    jp_block_t* block     = NULL;
    void* target_buffer   = NULL;
    unsigned char* buffer = NULL;
    jp_poller_t* poller   = jp_poller_create(100, &err);

    if (err) {
        goto clean_up;
    }

    buffer = jp_mem_calloc(1, ctx.chunk_size);
    err    = jp_poller_poll(poller, ctx.input_stream);
    if (err) {
        goto clean_up;
    }

    while (true) {
        err = jp_poller_wait(poller);
        if (JP_ATTR_UNLIKELY(err == JP_EREAD_FAILED)) {
            break;
        }
        if (err == JP_ETRYAGAIN) {
            continue;
        }
        while (true) {
            err = jp_queue_push_uncommitted(ctx.queue, &block);
            if (JP_ATTR_UNLIKELY(err == JP_ESHUTTING_DOWN)) {
                goto clean_up;
            }
            target_buffer = err == JP_EMSG_SHOULD_DROP ? buffer : block->data;
            read_len      = read(ctx.input_stream, target_buffer, ctx.chunk_size);
            if (JP_ATTR_UNLIKELY(read_len == 0)) {
                goto clean_up;
            }
            if (read_len < 0) {
                if (errno == EINTR) {
                    continue;
                }
                if (JP_ERRNO_EAGAIN(errno)) {
                    break;
                }
                err = JP_EREAD_FAILED;
                goto clean_up;
            }
            if (err == 0) {
                block->length = (size_t) read_len;
                jp_queue_push_commit(ctx.queue);
            }
        }
    }

clean_up:
    if (err == 0 || JP_QUEUE_IS_GRACEFUL_ERR(err)) {
        err = 0;
    }
    JP_FREE(buffer);
    jp_poller_destroy(poller);
    jp_queue_finalize(ctx.queue);
    return err;
}