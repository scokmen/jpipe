#ifndef JPIPE_JP_COMMON_H
#define JPIPE_JP_COMMON_H

#include <limits.h>

#if defined(__GNUC__)
#define JP_CONST_FUNC __attribute__((const))
#else
#define JP_CONST_FUNC
#endif

#ifdef PATH_MAX
#define JP_PATH_MAX PATH_MAX
#else
#define JP_PATH_MAX 4096
#endif

#define JP_FREE_IF_ALLOC(ptr)          \
do {                                   \
    if ((ptr) != NULL) {               \
        free((void*)(ptr));            \
        (ptr) = NULL;                  \
    }                                  \
} while (0)

#define JP_ALLOC_GUARD(var, expr)      \
do {                                   \
    (var) = (expr);                    \
    if ((var) == NULL) {               \
        jp_errno_log(JP_ENOMEM);       \
        return JP_ENOMEM;              \
    }                                  \
} while (0)

#define JP_ERROR_GUARD(stm)            \
do {                                   \
    jp_errno_t __err_val = (stm);      \
    if (__err_val) {                   \
       jp_errno_log(__err_val);        \
       return __err_val;               \
   }                                   \
} while (0)

#define JP_LOG_OUT(fmt, ...) \
fprintf(stdout, fmt "\n", ##__VA_ARGS__)

#define JP_LOG_ERR(fmt, ...) \
fprintf(stderr, "[jpipe]: Error: " fmt "\n", ##__VA_ARGS__)

#endif //JPIPE_JP_COMMON_H
