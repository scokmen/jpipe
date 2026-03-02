#ifndef JPIPE_JP_WORKER_H
#define JPIPE_JP_WORKER_H

#include <stdbool.h>
#include <jp_field.h>

#define BYTES_IN_KB            1024      // 1KB
#define JP_WRK_CHUNK_SIZE_MIN  1024      // 1KB
#define JP_WRK_CHUNK_SIZE_DEF  65536     // 64KB
#define JP_WRK_CHUNK_SIZE_MAX  131072    // 128KB
#define JP_WRK_BUFFER_SIZE_MIN 1
#define JP_WRK_BUFFER_SIZE_DEF 64
#define JP_WRK_BUFFER_SIZE_MAX 1024
#define JP_WRK_OUTDIR_DEF      "."
#define JP_WRK_FIELDS_MAX      32

JP_NONNULL_ARG(2)
JP_READ_PTR_SIZE(2, 1)
jp_errno_t jp_wrk_exec(int argc, char *argv[]);

#endif //JPIPE_JP_WORKER_H
