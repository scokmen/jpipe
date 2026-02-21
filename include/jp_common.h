#ifndef JPIPE_JP_COMMON_H
#define JPIPE_JP_COMMON_H

#include <limits.h>

#if defined(__GNUC__) || defined(__clang__)
#define JP_CONST_FUNC        __attribute__((const))
#define JP_PRINT_FUNC(fmt)   __attribute__((format(printf, fmt, (fmt) + 1)))
#define JP_UNUSED            __attribute__((unused))
#define JP_NONNULL_ARG(...)  __attribute__((nonnull(__VA_ARGS__)))
#define JP_MALLOC            __attribute__((malloc)) __attribute__((warn_unused_result))
#define JP_LIKELY(x)         __builtin_expect(!!(x), 1)
#define JP_UNLIKELY(x)       __builtin_expect(!!(x), 0)
#if defined(__clang__)
#define JP_ASSUME(cond)      __builtin_assume(cond)
#else
#define JP_ASSUME(cond)
#endif
#else
#error "Unsupported Compiler!"
#endif

#ifdef PATH_MAX
#define JP_PATH_MAX PATH_MAX
#else
#define JP_PATH_MAX 4096
#endif

#define JP_FREE_IF_ALLOC(ptr) \
do {                          \
    if ((ptr) != NULL) {      \
        free((void*)(ptr));   \
        (ptr) = NULL;         \
    }                         \
} while (0)

#define JP_ALLOC_GUARD(var, expr)           \
do {                                        \
    (var) = (expr);                         \
    if (JP_UNLIKELY((var) == NULL)) {       \
        return jp_errno_log_err(JP_ENOMEM); \
    }                                       \
} while (0)

#define JP_ALLOC_OR_RET(var, expr, ret) \
do {                                    \
    (var) = (expr);                     \
    if (JP_UNLIKELY((var) == NULL)) {   \
        return (ret);                   \
    }                                   \
} while (0)

#define JP_ERROR_GUARD(stm)       \
do {                              \
    jp_errno_t __err_val = (stm); \
    if (__err_val) {              \
       return __err_val;          \
   }                              \
} while (0)

#define JP_LOG_OUT(fmt, ...) \
fprintf(stdout, fmt "\n", ##__VA_ARGS__)

#endif //JPIPE_JP_COMMON_H
