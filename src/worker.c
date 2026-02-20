#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <getopt.h>
#include <sys/stat.h>
#include <unistd.h>
#include <jp_worker.h>
#include <jp_command.h>

static int display_help(void) {
    JP_LOG_OUT("Usage: jpipe run [options]\n");
    JP_LOG_OUT("Execute the data processing engine with the following configurations:\n");
    JP_LOG_OUT("Options:");
    JP_LOG_OUT("  -c, --chunk-size  <size>     Chunk size (e.g., 512kb, 16mb). Range: 1kb-64mb  (default: 1kb).");
    JP_LOG_OUT("  -b, --buffer-size <count>    Max pending operations. Range: 1-1024 (default: 64).");
    JP_LOG_OUT("  -o, --out-dir     <path>     Output directory (default: current dir).");
    JP_LOG_OUT("  -f, --field       key=value  Additional field to the JSON output. Can be used multiple times.");
    JP_LOG_OUT("  -h, --help                   Show this help message.\n");
    JP_LOG_OUT("Field Options:");
    JP_LOG_OUT("  -f, --field \"key=value\"   Add a field to the JSON output.\n");
    JP_LOG_OUT("  Key Rules:");
    JP_LOG_OUT("    - Must contain only: 'a-z', 'A-Z', '0-9', '_' and '-'.");
    JP_LOG_OUT("    - Maximum length: 64 characters.\n");
    JP_LOG_OUT("  Value Type Inference:");
    JP_LOG_OUT("    - key=123        -> Number  (no quotes in JSON).");
    JP_LOG_OUT("    - key=true|false -> Boolean (no quotes in JSON).");
    JP_LOG_OUT("    - key=string     -> String.");
    JP_LOG_OUT("    - key=\"string\"   -> String.");
    JP_LOG_OUT("    - key=\"123\"      -> Forced string.");
    JP_LOG_OUT("    - key=\"true\"     -> Forced string.");
    JP_LOG_OUT("    - key=\"str=ng\"   -> String.");
    JP_LOG_OUT("    - key=str=ng     -> Invalid field.\n");
    JP_LOG_OUT("  Example:");
    JP_LOG_OUT("    Input : jpipe -f \"id=101\" -f \"name=app\" -f \"active=true\" -f \"ver=1.2.0\"");
    JP_LOG_OUT("    Output: {\"id\": 101, \"name\": \"app\", \"active\": true, \"ver\": \"1.2.0\"}");
    return 0;
}

static int set_out_dir(const char *arg, jp_worker_args_t *args) {
    size_t len = 0;
    JP_FREE_IF_ALLOC(args->out_dir);
    if (arg == NULL) {
        return jp_errno_log_err_format(JP_EMISSING_CMD,
                                       "Output path is empty.");
    }
    len = strlen(arg);
    if (len == 0) {
        return jp_errno_log_err_format(JP_EMISSING_CMD,
                                       "Output path is empty.");
    }
    if (len > JP_PATH_MAX) {
        return jp_errno_log_err_format(JP_EMISSING_CMD,
                                       "Output path is too long. Maximum allowed path size: %d.", JP_PATH_MAX);
    }
    JP_ALLOC_GUARD(args->out_dir, strdup(arg));
    return 0;
}

static int set_chunk_size(const char *arg, jp_worker_args_t *args) {
    char *end_ptr;
    size_t chunk_size = 0;
    unsigned long long param = 0;

    errno = 0;
    param = strtoull(arg, &end_ptr, 10);

    if (errno == ERANGE) {
        return jp_errno_log_err_format(JP_ECHUNK_SIZE,
                                       "Chunk size format is incorrect: '%.32s'.", arg);
    }

    if (end_ptr == arg) {
        return jp_errno_log_err_format(JP_ECHUNK_SIZE,
                                       "Chunk size is empty: '%.32s'.", arg);
    }

    if (*end_ptr != '\0') {
        if (!strcmp(end_ptr, "kb") && param <= (JP_WRK_CHUNK_SIZE_MAX / BYTES_IN_KB)) {
            chunk_size += (param * BYTES_IN_KB);
        } else if (!strcmp(end_ptr, "mb") && param <= (JP_WRK_CHUNK_SIZE_MAX / BYTES_IN_MB)) {
            chunk_size += (param * BYTES_IN_MB);
        } else {
            return jp_errno_log_err_format(JP_ECHUNK_SIZE,
                                           "Chunk size value is invalid: '%.32s'.", arg);
        }
    }

    if (chunk_size < JP_WRK_CHUNK_SIZE_MIN || chunk_size > JP_WRK_CHUNK_SIZE_MAX) {
        return jp_errno_log_err_format(JP_ECHUNK_SIZE,
                                       "Chunk size value is invalid: '%.32s'.", arg);
    }
    args->chunk_size = (size_t) chunk_size;
    return 0;
}

static int set_buffer_size(const char *arg, jp_worker_args_t *args) {
    char *end_ptr;
    unsigned long long param = 0;

    errno = 0;
    param = strtoull(arg, &end_ptr, 10);

    if (errno == ERANGE || end_ptr == arg || *end_ptr != '\0') {
        return jp_errno_log_err_format(JP_EBUFFER_SIZE,
                                       "Buffer size format is incorrect: '%.32s'.", arg);
    }

    if (param < JP_WRK_BUFFER_SIZE_MIN || param > JP_WRK_BUFFER_SIZE_MAX) {
        return jp_errno_log_err_format(JP_EBUFFER_SIZE,
                                       "Buffer size format invalid: '%.32s'.", arg);
    }

    args->buffer_size = (size_t) param;
    return 0;
}

static int handle_unknown_argument(const char *cmd) {
    return jp_errno_log_err_format(JP_EUNKNOWN_RUN_CMD,
                                   "Invalid or incomplete [run] command: '%.32s'.", cmd);
}

static int get_field_args_count(int argc, char *argv[]) {
    int c = 0;
    for (int i = 0; i < argc; i++) {
        if (JP_CMD_EQ(argv[i], "-f", "--field")) {
            c++;
        }
    }
    return c;
}

static int init_worker_args(int argc, char *argv[], jp_worker_args_t *args) {
    int fields = get_field_args_count(argc, argv);
    if (fields > JP_WRK_FIELDS_MAX) {
        return jp_errno_log_err_format(JP_ETOO_MANY_FIELD,
                                       "Too many 'fields' specified: '%d'", fields);
    }
    JP_ALLOC_GUARD(args->fields, jp_field_set_new(fields));
    return 0;
}

static void free_worker_args(jp_worker_args_t *args) {
    JP_FREE_IF_ALLOC(args->out_dir);
    jp_field_set_free(args->fields);
}

static int collect_cli_args(int argc, char *argv[], jp_worker_args_t *args) {
    int option;
    opterr = 0;
    optind = 1;
    args->chunk_size = JP_WRK_CHUNK_SIZE_DEF;
    args->buffer_size = JP_WRK_BUFFER_SIZE_DEF;
    args->out_dir = NULL;

    static struct option long_options[] = {
            {"chunk-size" , required_argument, 0, 'c'},
            {"buffer-size", required_argument, 0, 'b'},
            {"field"      , required_argument, 0, 'f'},
            {"out-dir"    , required_argument, 0, 'o'},
            {"dry-run"    , required_argument, 0, 'n'},
            {"help"       , no_argument      , 0, 'h'},
            {0            , 0                , 0, 0  }
    };

    while ((option = getopt_long(argc, argv, ":c:b:o:f:hn", long_options, NULL)) != -1) {
        switch (option) {
            case 'c':
                JP_ERROR_GUARD(set_chunk_size(optarg, args));
                break;
            case 'b':
                JP_ERROR_GUARD(set_buffer_size(optarg, args));
                break;
            case 'o':
                JP_ERROR_GUARD(set_out_dir(optarg, args));
                break;
            case 'n':
                args->dry_run = true;
                break;
            case 'h':
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

static int create_and_normalize_out_dir(jp_worker_args_t *args) {
    char tmp[JP_PATH_MAX] = {0};
    char absolute_path[JP_PATH_MAX] = {0};
    char *p = NULL;
    struct stat st;

    size_t path_len = strlen(args->out_dir);
    strncpy(tmp, args->out_dir, sizeof(tmp));

    while (path_len > 1 && tmp[path_len - 1] == '/') {
        tmp[path_len - 1] = '\0';
        path_len--;
    }

    for (p = tmp + 1; *p; p++) {
        if (*p == '/') {
            *p = '\0';
            if (mkdir(tmp, 0755) != 0 && errno != EEXIST) {
                return jp_errno_log_err_format(JP_EOUT_DIR,
                                               "Could not create the output directory: '%s'", tmp);
            }
            *p = '/';
        }
    }

    if (mkdir(tmp, 0755) != 0 && errno != EEXIST) {
        return jp_errno_log_err_format(JP_EOUT_DIR,
                                       "Could not create the output directory: '%s'.", tmp);
    }


    if (stat(tmp, &st) != 0 || !S_ISDIR(st.st_mode)) {
        return jp_errno_log_err_format(JP_EOUT_DIR,
                                       "The target path is not a directory: '%s'.", tmp);
    }

    if (access(tmp, W_OK) != 0) {
        return jp_errno_log_err_format(JP_EOUT_DIR,
                                       "The target path is inaccessible: '%s'.", tmp);
    }

    if (realpath(tmp, absolute_path) == NULL) {
        return jp_errno_log_err_format(JP_EOUT_DIR,
                                       "Could not resolve absolute path: '%s'.", tmp);
    }

    JP_FREE_IF_ALLOC(args->out_dir);
    JP_ALLOC_GUARD(args->out_dir, strdup(absolute_path));
    return 0;
}

int jp_wrk_exec(int argc, char *argv[]) {
    if (argc == 2 && JP_CMD_EQ(argv[1], "-h", "--help")) {
        return display_help();
    }
    int err = 0;
    jp_worker_args_t args = {};
    
    err = init_worker_args(argc, argv, &args);
    if (err) {
        free_worker_args(&args);
        return err;
    }

    err = collect_cli_args(argc, argv, &args);
    if (err) {
        free_worker_args(&args);
        return err;
    }

    err = create_and_normalize_out_dir(&args);
    if (err) {
        free_worker_args(&args);
        return err;
    }

    free_worker_args(&args);
    return 0;
}
