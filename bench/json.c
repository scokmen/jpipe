#include <jp_bench.h>
#include <jp_common.h>
#include <jp_json.h>
#include <jp_memory.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

static void bench_json_value_encoder(const char* name, const char* filename, uint64_t iterations) {
    jp_bench_t bench;
    jp_bench_data_t* bench_data = jp_bench_data_prepare(filename);
    if (bench_data == NULL) {
        exit(EXIT_FAILURE);
    }

    const size_t dest_buffer_length = bench_data->max_chunk_size * 6;
    unsigned char* dest_buffer      = jp_mem_malloc(dest_buffer_length);
    
    for (uint64_t i = 0; i < 1000; i++) {
        const jp_bench_chunk_t chunk = bench_data->chunks[i % (uint64_t) bench_data->chunk_size];
        jp_json_value_encoder(chunk.buffer, chunk.length, dest_buffer, dest_buffer_length);
    }

    jp_bench_start(&bench, name, iterations);

    for (uint64_t i = 0; i < iterations; i++) {
        const jp_bench_chunk_t chunk = bench_data->chunks[i % (uint64_t) bench_data->chunk_size];
        jp_json_value_encoder(chunk.buffer, chunk.length, dest_buffer, dest_buffer_length);
    }

    jp_bench_stop(&bench);
    jp_bench_report(&bench);
    jp_bench_data_destroy(bench_data);
    JP_FREE(dest_buffer);
}

int main(void) {
    bench_json_value_encoder("Plain / Short", "short_plain_input.bin", 1000000);
    bench_json_value_encoder("Plain / Medium", "medium_plain_input.bin", 1000000);
    bench_json_value_encoder("Plain / Long", "long_plain_input.bin", 1000000);
    bench_json_value_encoder("Escaped / Short", "short_escaped_input.bin", 1000000);
    bench_json_value_encoder("Escaped / Medium", "medium_escaped_input.bin", 1000000);
    bench_json_value_encoder("Escaped / Long", "long_escaped_input.bin", 1000000);
    bench_json_value_encoder("Highly Escaped / Short", "short_highly_escaped_input.bin", 1000000);
    bench_json_value_encoder("Highly Escaped / Medium", "medium_highly_escaped_input.bin", 1000000);
    bench_json_value_encoder("Highly Escaped / Long", "long_highly_escaped_input.bin", 1000000);
    return EXIT_SUCCESS;
}
