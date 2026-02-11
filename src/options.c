#include <getopt.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <j_errno.h>
#include <j_options.h>

int read_chunk_size(const char *arg, size_t *size) {
    char *end_ptr;
    size_t chunk_size = 1;
    unsigned long long param = 0;

    errno = 0;
    param = strtoull(arg, &end_ptr, 10);

    if (end_ptr == arg || errno == ERANGE) {
        return J_ECHUNK_SIZE;
    }

    if (*end_ptr != '\0') {
        if (!strcmp(end_ptr, "kb") && param <= (J_CHUNK_SIZE_MAX / BYTES_IN_KB)) {
            chunk_size = (param * BYTES_IN_KB);
        } else if (!strcmp(end_ptr, "mb") && param <= (J_CHUNK_SIZE_MAX / BYTES_IN_MB)) {
            chunk_size = (param * BYTES_IN_MB);
        } else {
            return J_ECHUNK_SIZE;
        }
    }

    if (chunk_size < J_CHUNK_SIZE_MIN || chunk_size > J_CHUNK_SIZE_MAX) {
        return J_ECHUNK_SIZE;
    }
    *size = (size_t) chunk_size;
    return 0;
}

int read_backlog_len(const char *arg, size_t *size) {
    char *end_ptr;
    unsigned long long param = 0;

    errno = 0;
    param = strtoull(arg, &end_ptr, 10);

    if (end_ptr == arg || *end_ptr != '\0' || errno == ERANGE || param > J_BACKLOG_LEN_MAX) {
        return J_EBACKLOG_LENGTH;
    }

    *size = (size_t) param;
    return 0;
}

int read_out_dir(const char *arg, const char **dir) {
    if (arg == NULL || strlen(arg) == 0) {
        return J_EOUT_DIR;
    }
    *dir = strdup(arg);
    return 0;
}

int jp_options_init(int argc, char *argv[], jp_options_t *opts) {
    int opt;
    opts->backlog_len = J_BACKLOG_LEN_DEF;
    opts->chunk_size = J_CHUNK_SIZE_DEF;
    opts->out_dir = J_OUTDIR_DEF;

    static struct option long_options[] = {
            {"chunk-size", required_argument, 0, 'c'},
            {"backlog",    required_argument, 0, 'b'},
            {"out-dir",    required_argument, 0, 'o'},
            {0, 0,                            0, 0}
    };

    while ((opt = getopt_long(argc, argv, "c:b:o:", long_options, NULL)) != -1) {
        switch (opt) {
            case 'c':
                EXPLAIN_IF_FAILS(read_chunk_size(optarg, &opts->chunk_size), optarg);
                break;
            case 'b':
                EXPLAIN_IF_FAILS(read_backlog_len(optarg, &opts->backlog_len), optarg);
                break;
            case 'o':
                EXPLAIN_IF_FAILS(read_out_dir(optarg, &opts->out_dir), optarg);
                break;    
            default: {

            }
        }
    }
    return 0;
}