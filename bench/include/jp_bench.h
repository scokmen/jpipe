#ifndef JPIPE_JP_BENCH_H
#define JPIPE_JP_BENCH_H

#include <jp_common.h>
#include <stdint.h>
#include <time.h>

typedef struct {
    struct timespec start;
    struct timespec end;
    uint64_t total_ns;
    uint64_t iterations;
    const char* name;
} jp_bench_t;

JP_ATTR_NONNULL(1, 2)
JP_ATTR_READ_ONLY(2)
void jp_bench_start(jp_bench_t* bench, const char* name, uint64_t iterations);

JP_ATTR_NONNULL(1)
void jp_bench_stop(jp_bench_t* bench);

JP_ATTR_NONNULL(1)
void jp_bench_report(jp_bench_t* bench);

typedef struct {
    unsigned char* buffer;
    size_t length;
} jp_bench_chunk_t;

typedef struct {
    size_t chunk_size;
    size_t total_size;
    size_t max_chunk_size;
    jp_bench_chunk_t* chunks;
    void* data;
} jp_bench_data_t;

JP_ATTR_NONNULL(1)
JP_ATTR_READ_ONLY(1)
JP_ATTR_ALLOCATED
JP_ATTR_USE_RETURN
jp_bench_data_t* jp_bench_data_prepare(const char* filename);

void jp_bench_data_destroy(jp_bench_data_t* bench_data);

#endif  // JPIPE_JP_BENCH_H
