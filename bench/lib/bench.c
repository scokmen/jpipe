#include <fcntl.h>
#include <inttypes.h>
#include <jp_bench.h>
#include <jp_memory.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

void jp_bench_start(jp_bench_t* bench, const char* name, uint64_t iterations) {
    bench->name       = name;
    bench->iterations = iterations;
    bench->total_ns   = 0;
    clock_gettime(CLOCK_MONOTONIC, &bench->start);
}

void jp_bench_stop(jp_bench_t* bench) {
    clock_gettime(CLOCK_MONOTONIC, &bench->end);

    int64_t seconds     = bench->end.tv_sec - bench->start.tv_sec;
    int64_t nanoseconds = bench->end.tv_nsec - bench->start.tv_nsec;

    if (nanoseconds < 0) {
        seconds--;
        nanoseconds += 1000000000L;
    }

    bench->total_ns = (uint64_t) seconds * 1000000000ULL + (uint64_t) nanoseconds;
}

void jp_bench_report(jp_bench_t* bench) {
    const double total_ms = (double) bench->total_ns / 1000000.0;
    const double avg_ns   = (double) bench->total_ns / (double) bench->iterations;
    const double ops_sec  = (double) bench->iterations * 1000000000.0 / (double) bench->total_ns;

    fprintf(stdout, "-------------------------------\n");
    fprintf(stdout, "Benchmark              : %.128s\n", bench->name);
    fprintf(stdout, "-------------------------------\n");
    fprintf(stdout, " - Iterations          : %" PRIu64 "\n", bench->iterations);
    fprintf(stdout, " - Elapsed Time (ms)   : %.3f\n", total_ms);
    fprintf(stdout, " - Average (ns/op)     : %.2f\n", avg_ns);
    fprintf(stdout, " - Operation (ops/sec) : %.2f\n", ops_sec);
}

jp_bench_data_t* jp_bench_data_prepare(const char* filename) {
    struct stat st;
    char file_path[JP_PATH_MAX];
    jp_bench_data_t* bench_data = jp_mem_malloc(sizeof(jp_bench_data_t));

    snprintf(file_path, JP_PATH_MAX, "%s/%s", JP_BENCH_DATA_DIR, filename);
    const int fd = open(file_path, O_RDONLY);
    if (fd < 0) {
        perror("File cannot be opened!");
        return NULL;
    }

    if (fstat(fd, &st) < 0) {
        perror("File stat cannot be obtained!");
        close(fd);
        return NULL;
    }

    bench_data->total_size = (size_t) st.st_size;
    bench_data->data       = mmap(NULL, bench_data->total_size, PROT_READ, MAP_PRIVATE, fd, 0);
    close(fd);

    if (bench_data->data == MAP_FAILED) {
        perror("Memory mapping failed!");
        JP_FREE(bench_data);
        return NULL;
    }

    madvise(bench_data->data, bench_data->total_size, MADV_SEQUENTIAL);
    size_t capacity        = 0;
    const unsigned char* p = bench_data->data;
    for (size_t i = 0; i < bench_data->total_size; i++) {
        if (p[i] == 0x1E) {
            capacity++;
        }
    }

    bench_data->chunks     = jp_mem_malloc(sizeof(jp_bench_chunk_t) * capacity);
    bench_data->chunk_size = 0;

    unsigned char* start   = bench_data->data;
    unsigned char* end     = start + bench_data->total_size;
    unsigned char* current = start;

    bench_data->max_chunk_size = 0;
    while (current < end && bench_data->chunk_size < capacity) {
        unsigned char* delimiter = memchr(current, 0x1E, (size_t) (end - current));
        if (delimiter == NULL) {
            break;
        }
        const size_t chunk_size = (size_t) (delimiter - current);
        if (chunk_size > bench_data->max_chunk_size) {
            bench_data->max_chunk_size = chunk_size;
        }
        bench_data->chunks[bench_data->chunk_size].buffer = current;
        bench_data->chunks[bench_data->chunk_size].length = chunk_size;
        current                                           = delimiter + 1;
        bench_data->chunk_size++;
    }

    return bench_data;
}

void jp_bench_data_destroy(jp_bench_data_t* bench_data) {
    if (bench_data == NULL) {
        return;
    }
    if (bench_data->data != NULL) {
        munmap(bench_data->data, bench_data->total_size);
    }
    if (bench_data->chunks != NULL) {
        JP_FREE(bench_data->chunks);
    }
    JP_FREE(bench_data);
}