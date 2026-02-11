#ifndef JPIPE_J_COMMON_H
#define JPIPE_J_COMMON_H

#if defined(__GNUC__)
    #define J_CONST_FUNC __attribute__((const))
#else
    #define J_CONST_FUNC
#endif

#define RETURN_IF_FAILS(stm) do {                                   \
                                 j_errno_t __err_val = (stm);       \
                                 if (__err_val) {                   \
                                     return __err_val;              \
                                 }                                  \
                             } while (0);
                               
#endif //JPIPE_J_COMMON_H
