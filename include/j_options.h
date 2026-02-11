#ifndef JPIPE_J_OPTIONS_H
#define JPIPE_J_OPTIONS_H

#include <stdlib.h>

#define BYTES_IN_KB    1024      // 1KB
#define BYTES_IN_MB    1048576   // 1MB
#define J_BUF_MIN_SIZE 1024      // 1KB
#define J_BUF_DEF_SIZE 65536     // 64KB
#define J_BUF_MAX_SIZE 67108864  // 64MB
#define J_QUE_MIN_SIZE 0
#define J_QUE_DEF_SIZE 0
#define J_QUE_MAX_SIZE 1024

typedef struct {
    size_t buf_size;
    size_t que_size;
} jp_options_t;

int jp_options_init(int argc, char *argv[], jp_options_t *opts);

#endif //JPIPE_J_OPTIONS_H
