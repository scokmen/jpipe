#ifndef JPIPE_J_ERRNO_H
#define JPIPE_J_ERRNO_H

#include <j_common.h>

#define J_ERRNO_MAP(XX)                                                                                            \
  XX(ECHUNK_SIZE    , "[-c, --chunk-size] Chunk size must be between 1KB (65536 bytes) and 64MB (67108864 bytes)") \
  XX(EBACKLOG_LENGTH, "[-b, --backlog   ] Backlog length must be between 0 and 65536")                             \
  XX(EOUT_DIR       , "[-o, --out-dir   ] Output directory is invalid")                               

typedef enum {
    J_OK = 0,
    #define XX(code, _) J_##code,
        J_ERRNO_MAP(XX)
    #undef XX
} j_errno_t;

const char *j_errno_explain(j_errno_t err) J_CONST_FUNC;

#endif //JPIPE_J_ERRNO_H
