#ifndef JPIPE_J_OPTIONS_H
#define JPIPE_J_OPTIONS_H

#include <stdio.h>
#include <j_errno.h>

#define BYTES_IN_KB       1024      // 1KB
#define BYTES_IN_MB       1048576   // 1MB
#define J_CHUNK_SIZE_MIN  1024      // 1KB
#define J_CHUNK_SIZE_DEF  65536     // 64KB
#define J_CHUNK_SIZE_MAX  67108864  // 64MB
#define J_BACKLOG_LEN_MIN 0
#define J_BACKLOG_LEN_DEF 0
#define J_BACKLOG_LEN_MAX 1024
#define J_OUTDIR_DEF      "."

#define EXPLAIN_IF_FAILS(stm, option)  \
do {                                                                      \
    j_errno_t __err_val = (stm);                                          \
    if (__err_val) {                                                      \
       fprintf(stderr, "[jpipe]: invalid option '%.256s'\n", (option));   \
       fprintf(stderr, "[jpipe]: %.256s\n", j_errno_explain(__err_val));  \
       return __err_val;                                                  \
   }                                                                      \
} while (0)

typedef struct {
    size_t chunk_size;
    size_t backlog_len;
    const char* out_dir;
} jp_options_t;

int jp_options_init(int argc, char *argv[], jp_options_t *opts);

#endif //JPIPE_J_OPTIONS_H
