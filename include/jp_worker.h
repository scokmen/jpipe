#ifndef JPIPE_JP_WORKER_H
#define JPIPE_JP_WORKER_H

#define JP_WRK_CHUNK_SIZE_MIN  1024    // 1KB
#define JP_WRK_CHUNK_SIZE_DEF  16384   // 16KB
#define JP_WRK_CHUNK_SIZE_MAX  131072  // 128KB
#define JP_WRK_BUFFER_SIZE_MIN 1
#define JP_WRK_BUFFER_SIZE_DEF 64
#define JP_WRK_BUFFER_SIZE_MAX 1024
#define JP_WRK_OUTDIR_DEF      "."
#define JP_WRK_FIELDS_MAX      32

JP_ATTR_NONNULL(2)
JP_ATTR_READONLY_N(2, 1)
jp_errno_t jp_wrk_exec(int argc, char* argv[]);

#endif  // JPIPE_JP_WORKER_H
