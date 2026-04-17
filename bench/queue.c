#include <jp_bench.h>
#include <jp_memory.h>
#include <jp_queue.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    uint64_t iterations;
    jp_queue_t* queue;
} bench_ctx_t;

static void* data_reader(void* data) {
    jp_errno_t err         = 0;
    jp_block_t* block      = NULL;
    const bench_ctx_t* ctx = data;
    jp_queue_t* queue      = ctx->queue;

    while (true) {
        err = jp_queue_pop_uncommitted(queue, &block);
        if (err) {
            break;
        }
        jp_queue_pop_commit(queue);
    }
    pthread_exit(NULL);
}

static void* data_writer(void* data) {
    jp_errno_t err            = 0;
    const bench_ctx_t* ctx    = data;
    jp_block_t* block         = NULL;
    jp_queue_t* queue         = ctx->queue;
    const uint64_t iterations = ctx->iterations;

    for (uint64_t i = 0; i < iterations; i++) {
        err = jp_queue_push_uncommitted(queue, &block);
        if (err || block == NULL) {
            break;
        }
        block->data[0] = 65;
        block->length  = 1;
        jp_queue_push_commit(queue);
    }
    jp_queue_finalize(queue);
    pthread_exit(NULL);
}

static void bench_queue_read_write(const char* name, size_t capacity, uint64_t iterations) {
    jp_bench_t bench;
    jp_errno_t err    = 0;
    jp_queue_t* queue = jp_queue_create(capacity, BYTES_IN_KB, JP_QUEUE_POLICY_WAIT, &err);
    if (err || queue == NULL) {
        exit(EXIT_FAILURE);
    }

    pthread_t reader, writer;
    bench_ctx_t bench_data = {.iterations = iterations, .queue = queue};

    jp_bench_start(&bench, name, iterations);

    pthread_create(&reader, NULL, data_reader, &bench_data);
    pthread_create(&writer, NULL, data_writer, &bench_data);

    pthread_join(reader, NULL);
    pthread_join(writer, NULL);

    jp_bench_stop(&bench);
    jp_bench_report(&bench);
    jp_queue_destroy(queue);
}

int main(void) {
    bench_queue_read_write("Warm-Up: 1", 128, 10000000);
    bench_queue_read_write("Warm-Up: 2", 128, 10000000);
    bench_queue_read_write("Warm-Up: 3", 128, 10000000);
    bench_queue_read_write("Queue Capacity: 8", 8, 10000000);
    bench_queue_read_write("Queue Capacity: 32", 32, 10000000);
    bench_queue_read_write("Queue Capacity: 256", 256, 10000000);
    bench_queue_read_write("Queue Capacity: 1024", 1024, 10000000);
    return EXIT_SUCCESS;
}
