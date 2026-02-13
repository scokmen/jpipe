#ifndef JPIPE_JP_WORKER_H
#define JPIPE_JP_WORKER_H

#define BYTES_IN_KB            1024      // 1KB
#define BYTES_IN_MB            1048576   // 1MB
#define JP_WRK_CHUNK_SIZE_MIN  1024      // 1KB
#define JP_WRK_CHUNK_SIZE_DEF  65536     // 64KB
#define JP_WRK_CHUNK_SIZE_MAX  67108864  // 64MB
#define JP_WRK_BACKLOG_LEN_MIN 1
#define JP_WRK_BACKLOG_LEN_DEF 1
#define JP_WRK_BACKLOG_LEN_MAX 1024
#define JP_WRK_OUTDIR_DEF      "."

typedef struct {
    size_t chunk_size;
    size_t backlog_len;
    const char *out_dir;
} jp_worker_args_t;

int jp_wrk_exec(int argc, char *argv[]);

#endif //JPIPE_JP_WORKER_H
