#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <getopt.h>
#include <jp_worker.h>
#include <jp_command.h>

static int display_help() {
    JP_LOG_OUT("Usage: jpipe run [options]\n");
    JP_LOG_OUT("Execute the data processing engine with the following configurations:\n");
    JP_LOG_OUT("Options:");
    JP_LOG_OUT("  -c, --chunk-size <size>    Buffer size (e.g., 512kb, 16mb). Range: 1kb-64mb  (default: 1kb)");
    JP_LOG_OUT("  -b, --backlog    <count>   Max pending operations. Range: 1-1024 (default: 1)");
    JP_LOG_OUT("  -o, --out-dir    <path>    Output directory.");
    JP_LOG_OUT("  -h, --help                 Show this help message.");
    return 0;
}

static int set_out_dir(const char *arg, jp_worker_args_t *args) {
    JP_FREE_IF_ALLOC(args->out_dir);
    if (arg == NULL || strlen(arg) == 0) {
        return JP_EOUT_DIR;
    }
    JP_ALLOC_GUARD(args->out_dir, strdup(arg));
    return 0;
}

static int set_chunk_size(const char *arg, jp_worker_args_t *args) {
    char *end_ptr;
    size_t chunk_size = 1;
    unsigned long long param = 0;

    errno = 0;
    param = strtoull(arg, &end_ptr, 10);

    if (end_ptr == arg || errno == ERANGE) {
        return JP_ECHUNK_SIZE;
    }

    if (*end_ptr != '\0') {
        if (!strcmp(end_ptr, "kb") && param <= (JP_WRK_CHUNK_SIZE_MAX / BYTES_IN_KB)) {
            chunk_size = (param * BYTES_IN_KB);
        } else if (!strcmp(end_ptr, "mb") && param <= (JP_WRK_CHUNK_SIZE_MAX / BYTES_IN_MB)) {
            chunk_size = (param * BYTES_IN_MB);
        } else {
            return JP_ECHUNK_SIZE;
        }
    }

    if (chunk_size < JP_WRK_CHUNK_SIZE_MIN || chunk_size > JP_WRK_CHUNK_SIZE_MAX) {
        return JP_ECHUNK_SIZE;
    }
    args->chunk_size = (size_t) chunk_size;
    return 0;
}

static int set_backlog_len(const char *arg, jp_worker_args_t *args) {
    char *end_ptr;
    unsigned long long param = 0;

    errno = 0;
    param = strtoull(arg, &end_ptr, 10);

    if (end_ptr == arg || *end_ptr != '\0' || errno == ERANGE || param < JP_WRK_BACKLOG_LEN_MIN || param > JP_WRK_BACKLOG_LEN_MAX) {
        return JP_EBACKLOG_LENGTH;
    }

    args->backlog_len = (size_t) param;
    return 0;
}

static int handle_unknown_argument(const char *cmd) {
    jp_cmd_invalid(cmd);
    return JP_EUNKNOWN_CMD;
}

static int set_arguments(int argc, char *argv[], jp_worker_args_t *args) {
    int option;
    opterr = 0;
    optind = 1;
    args->chunk_size = JP_WRK_CHUNK_SIZE_DEF;
    args->backlog_len = JP_WRK_BACKLOG_LEN_DEF;
    args->out_dir = NULL;

    static struct option long_options[] = {
            {"chunk-size", required_argument, 0, 'c'},
            {"backlog",    required_argument, 0, 'b'},
            {"out-dir",    required_argument, 0, 'o'},
            {0, 0,                            0, 0}
    };

    while ((option = getopt_long(argc, argv, ":c:b:o:", long_options, NULL)) != -1) {
        switch (option) {
            case 'c':
                JP_ERROR_GUARD(set_chunk_size(optarg, args));
                break;
            case 'b':
                JP_ERROR_GUARD(set_backlog_len(optarg, args));
                break;
            case 'o':
                JP_ERROR_GUARD(set_out_dir(optarg, args));
                break;
            case ':':
            case '?':
                JP_ERROR_GUARD(handle_unknown_argument(argv[optind - 1]));
                break;
            default: {

            }
        }
    }

    if (optind < argc) {
        JP_ERROR_GUARD(handle_unknown_argument(argv[optind]));
    }

    if (args->out_dir == NULL) {
        JP_ALLOC_GUARD(args->out_dir, strdup(JP_WRK_OUTDIR_DEF));
    }

    return 0;
}

int jp_wrk_exec(int argc, char *argv[]) {
    if (argc == 2 && JP_CMD_EQ(argv[1], "-h", "--help")) {
        return display_help();
    }
    int err = 0;
    jp_worker_args_t args = {};
    
    err = set_arguments(argc, argv, &args);
    if (err) {
        JP_FREE_IF_ALLOC(args.out_dir);
        return err;
    }

    JP_FREE_IF_ALLOC(args.out_dir);
    return 0;
}
