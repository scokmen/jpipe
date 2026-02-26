#ifndef JPIPE_JP_COMMON_H
#define JPIPE_JP_COMMON_H

#define JP_PATH_MAX          4096
#define MIN(x, y)            (((x) < (y)) ? (x) : (y))
#define MAX(x, y)            (((x) > (y)) ? (x) : (y))
#define JP_LOG_OUT(fmt, ...) fprintf(stdout, fmt "\n", ##__VA_ARGS__)

/*
 * Checks if __has_builtin is defined or not.
 */
#ifdef __has_builtin
#define HAS_BUILTIN(x) __has_builtin(x)
#else
#define HAS_BUILTIN(x) (0)
#endif

/*
 * Checks if __has_attribute is defined or not.
 */
#ifdef __has_attribute
#define HAS_ATTRIBUTE(x) __has_attribute(x)
#else
#define HAS_ATTRIBUTE(x) (0)
#endif


/*
 * Defines: JP_ASSUME
 */
#if HAS_BUILTIN(__builtin_assume)
#define JP_ASSUME(cond) __builtin_assume(cond)
#elif HAS_BUILTIN(__builtin_unreachable)
#define JP_ASSUME(cond) do { if (!(cond)) __builtin_unreachable(); } while (0)
#else
#define JP_ASSUME(cond)
#endif

/*
 * Defines: JP_LIKELY, JP_UNLIKELY
 */
#if HAS_BUILTIN(__builtin_expect)
#define JP_LIKELY(x)    __builtin_expect(!!(x), 1)
#define JP_UNLIKELY(x)  __builtin_expect(!!(x), 0)
#else
#define JP_LIKELY(x)    (x)
#define JP_UNLIKELY(x)  (x)
#endif

/*
 * Defines: JP_MALLOC
 */
#if HAS_ATTRIBUTE(malloc)
#define JP_MALLOC __attribute__((malloc))
#else
#define JP_MALLOC
#endif

/*
 * Defines: JP_USE_RESULT
 */
#if HAS_ATTRIBUTE(warn_unused_result)
#define JP_USE_RESULT __attribute__((warn_unused_result))
#else
#define JP_USE_RESULT
#endif

/*
 * Defines: JP_CONST_FUNC
 */
#if HAS_ATTRIBUTE(const)
#define JP_CONST_FUNC __attribute__((const))
#else
#define JP_CONST_FUNC
#endif

/*
 * Defines: JP_PRINT_FUNC
 */
#if HAS_ATTRIBUTE(format)
#define JP_PRINT_FUNC(fmt) __attribute__((format(printf, fmt, (fmt) + 1)))
#else
#define JP_PRINT_FUNC
#endif

/*
 * Defines: JP_UNUSED
 */
#if HAS_ATTRIBUTE(unused)
#define JP_UNUSED __attribute__((unused))
#else
#define JP_UNUSED
#endif

/*
 * Defines: JP_NONNULL_ARG
 */
#if HAS_ATTRIBUTE(nonnull)
#define JP_NONNULL_ARG(...) __attribute__((nonnull(__VA_ARGS__)))
#else
#define JP_NONNULL_ARG(...)
#endif

/*
 * Defines: JP_READ_PTR, JP_READ_PTR_SIZE, JP_WRITE_PTR, JP_WRITE_PTR_SIZE
 */
#if HAS_ATTRIBUTE(access)
#define JP_READ_PTR(ptr_idx)                 __attribute__((access(read_only, ptr_idx)))
#define JP_READ_PTR_SIZE(ptr_idx, size_idx)  __attribute__((access(read_only, ptr_idx, size_idx)))
#define JP_WRITE_PTR(ptr_idx)                __attribute__((access(write_only, ptr_idx)))
#define JP_WRITE_PTR_SIZE(ptr_idx, size_idx) __attribute__((access(write_only, ptr_idx, size_idx)))
#else
#define JP_READ_PTR(ptr_idx)
#define JP_READ_PTR_SIZE(ptr_idx, size_idx)
#define JP_WRITE_PTR(ptr_idx)
#define JP_WRITE_PTR_SIZE(ptr_idx, size_idx)
#endif

#define JP_FREE(ptr)        \
do {                        \
    if ((ptr) != NULL) {    \
        free((void*)(ptr)); \
        (ptr) = NULL;       \
    }                       \
} while (0)

#define JP_ALLOC_OR_RET(var, expr, ret) \
do {                                    \
    (var) = (expr);                     \
    if (JP_UNLIKELY((var) == NULL)) {   \
        return (ret);                   \
    }                                   \
} while (0)

#define JP_OK_OR_RET(stm)          \
do {                               \
    __typeof__(stm) __err = (stm); \
    if (__err != 0) {              \
       return __err;               \
    }                              \
} while (0)

#endif //JPIPE_JP_COMMON_H
