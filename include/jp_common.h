#ifndef JPIPE_JP_COMMON_H
#define JPIPE_JP_COMMON_H

#if defined(__GNUC__)
#define JP_CONST_FUNC __attribute__((const))
#else
#define JP_CONST_FUNC
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

#define JP_ERROR_GUARD(stm, option)    \
do {                                   \
    jp_errno_t __err_val = (stm);      \
    if (__err_val) {                   \
       jp_errno_log(__err_val);        \
       return __err_val;               \
   }                                   \
} while (0)

#endif //JPIPE_JP_COMMON_H
