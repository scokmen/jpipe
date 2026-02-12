#ifndef JPIPE_J_COMMON_H
#define JPIPE_J_COMMON_H

#if defined(__GNUC__)
    #define J_CONST_FUNC __attribute__((const))
#else
    #define J_CONST_FUNC
#endif

#define FREE_IF_ALLOC(ptr)  \
do {                        \
    if ((ptr) != NULL) {    \
        free((void*)(ptr)); \
        (ptr) = NULL;       \
    }                       \
} while (0)

#endif //JPIPE_J_COMMON_H
