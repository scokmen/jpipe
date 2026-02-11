#ifndef JPIPE_J_ERRNO_H
#define JPIPE_J_ERRNO_H

#include <j_common.h>

#define J_ERRNO_MAP(XX)                                                                              \
  XX(EBUFFER_SIZE , "Buffer size must be between 1KB (65536 bytes) and 64MB (67108864 bytes)")       \
  XX(EQUEUE_LENGTH, "Queue length must be between 0 and 65536")

typedef enum {
    J_OK = 0,
    #define XX(code, _) J_##code,
        J_ERRNO_MAP(XX)
    #undef XX
} j_errno_t;

const char *j_errno_explain(j_errno_t errno) J_CONST_FUNC;

#endif //JPIPE_J_ERRNO_H
