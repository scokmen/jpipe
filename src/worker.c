#include <getopt.h>
#include <jp_command.h>
#include <jp_errno.h>
#include <jp_field.h>
#include <jp_queue.h>
#include <jp_reader.h>
#include <jp_worker.h>
#include <jp_writer.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

typedef void* (*worker_func)(void*);

typedef struct {
    bool running;
    bool detached;
    pthread_t tid;
    worker_func func;
} worker_thread_t;

typedef struct {
    int input_stream;
    bool dry_run;
    size_t chunk_size;
    size_t buffer_size;
    jp_queue_policy_t policy;
    jp_queue_t* queue;
    jp_field_set_t* fields;
    const char* out_dir;
} worker_ctx_t;

static jp_errno_t display_help(void) {
    JP_LOG_INFO("Usage: jpipe run [options]");
    JP_LOG_INFO("\nExecute the data processing engine with the following configurations:");
    JP_LOG_INFO("\nOptions:");
    JP_LOG_INFO("  -c, --chunk-size  <size>     Chunk size (e.g., 16kb, 64kb). Range: 1kb-128kb  (default: 16kb).");
    JP_LOG_INFO("  -b, --buffer-size <count>    Max pending operations. Range: 1-1024 (default: 64).");
    JP_LOG_INFO("  -p, --policy      <type>     Overflow policy: 'wait' or 'drop' (default: wait).");
    JP_LOG_INFO("  -o, --output      <path>     Output directory (default: current dir).");
    JP_LOG_INFO("  -f, --field       key=value  Additional field to the JSON output. Can be used multiple times.");
    JP_LOG_INFO("  -n, --dry-run                Dry run.");
    JP_LOG_INFO("  -h, --help                   Show this help message.");
    JP_LOG_INFO("\nField Options:");
    JP_LOG_INFO("  -f, --field \"key=value\"   Add a field to the JSON output.");
    JP_LOG_INFO("\n  Key Rules:");
    JP_LOG_INFO("    - Must contain only: 'a-z', 'A-Z', '0-9', '_' and '-'.");
    JP_LOG_INFO("    - Maximum length: 64 characters.");
    JP_LOG_INFO("\n  Value Type Inference:");
    JP_LOG_INFO("    - key=123        -> Number  (no quotes in JSON).");
    JP_LOG_INFO("    - key=true|false -> Boolean (no quotes in JSON).");
    JP_LOG_INFO("    - key=string     -> String.");
    JP_LOG_INFO("    - key=\"string\"   -> String.");
    JP_LOG_INFO("    - key=\"123\"      -> Forced string.");
    JP_LOG_INFO("    - key=\"true\"     -> Forced string.");
    JP_LOG_INFO("    - key=\"str=ng\"   -> String.");
    JP_LOG_INFO("    - key=str=ng     -> Invalid field.");
    JP_LOG_INFO("\n  Example:");
    JP_LOG_INFO("    Input : jpipe -f \"id=101\" -f \"name=app\" -f \"active=true\" -f \"ver=1.2.0\"");
    JP_LOG_INFO("    Output: {\"id\": 101, \"name\": \"app\", \"active\": true, \"ver\": \"1.2.0\"}");
    return 0;
}

static void display_summary(worker_ctx_t* ctx) {
    double estimated_mem_usage = (double) ctx->chunk_size * (double) ctx->buffer_size / (BYTES_IN_KB * BYTES_IN_KB);

    JP_LOG_MSG("Application is starting...\n");
    JP_LOG_MSG("[Runtime Parameters]");
    JP_LOG_MSG("• Chunk Size   (-c) : %zu KB", (ctx->chunk_size / BYTES_IN_KB));
    JP_LOG_MSG("• Buffer Size  (-b) : %zu", ctx->buffer_size);
    JP_LOG_MSG("• Output Dir   (-o) : %s", ctx->out_dir);
    JP_LOG_MSG("• Policy       (-p) : %s", ctx->policy == JP_QUEUE_POLICY_WAIT ? "WAIT" : "DROP");
    if (ctx->fields->len > 0) {
        JP_LOG_MSG("• Fields       (-f) :");
        for (size_t i = 0; i < ctx->fields->len; i++) {
            JP_LOG_MSG("     %zu. %-32s= %-32s", i + 1, ctx->fields->fields[i]->key, ctx->fields->fields[i]->val);
        }
    }
    JP_LOG_MSG("\n[Resource Utilization]");
    JP_LOG_MSG("• Memory Usage      :  ~%.2f MB", estimated_mem_usage);
    JP_LOG_MSG("\n* These values are based on user-provided parameters.");
    JP_LOG_MSG(
        "* Memory usage is an approximation;"
        "operating system overhead and thread stack allocations are not included.\n");
}

static jp_errno_t set_out_dir(const char* arg, worker_ctx_t* ctx) {
    size_t len = 0;
    JP_FREE(ctx->out_dir);
    if (arg == NULL) {
        return jp_errno_log_err_format(JP_EMISSING_CMD, "Path is empty.");
    }
    len = strlen(arg);
    if (len == 0) {
        return jp_errno_log_err_format(JP_EMISSING_CMD, "Path is empty.");
    }
    if (len > JP_PATH_MAX) {
        return jp_errno_log_err_format(JP_EMISSING_CMD, "Path is too long. Maximum allowed length: %d.", JP_PATH_MAX);
    }
    JP_ERRNO_ALLOC(ctx->out_dir, strdup(arg));
    return 0;
}

static jp_errno_t set_field(const char* arg, worker_ctx_t* ctx) {
    jp_errno_t err;
    if (arg == NULL) {
        return jp_errno_log_err_format(JP_EINV_FIELD_KEY, "Field is invalid.");
    }

    JP_ATTR_ASSUME(ctx->fields != NULL);
    err = jp_field_set_add(ctx->fields, arg);
    if (err) {
        return jp_errno_log_err_format(err, "Field is invalid: '%s'", arg);
    }
    return 0;
}

static jp_errno_t set_chunk_size(const char* arg, worker_ctx_t* ctx) {
    char* end_ptr;
    size_t chunk_size        = 0;
    unsigned long long param = 0;

    errno = 0;
    param = strtoull(arg, &end_ptr, 10);

    if (errno == ERANGE || errno == EINVAL) {
        return jp_errno_log_err_format(JP_ECHUNK_SIZE, "Size is invalid: '%.32s'.", arg);
    }

    if (end_ptr == arg) {
        return jp_errno_log_err_format(JP_ECHUNK_SIZE, "Size is invalid: '%.32s'.", arg);
    }

    if (*end_ptr != '\0' && !strcmp(end_ptr, "kb") && param <= JP_CONF_CHUNK_SIZE_MAX / BYTES_IN_KB) {
        chunk_size += param * BYTES_IN_KB;
    } else {
        return jp_errno_log_err_format(JP_ECHUNK_SIZE, "Size is invalid: '%.32s'.", arg);
    }

    if (chunk_size < JP_CONF_CHUNK_SIZE_MIN || chunk_size > JP_CONF_CHUNK_SIZE_MAX) {
        return jp_errno_log_err_format(JP_ECHUNK_SIZE, "Size is invalid: '%.32s'.", arg);
    }
    ctx->chunk_size = chunk_size;
    return 0;
}

static jp_errno_t set_buffer_size(const char* arg, worker_ctx_t* ctx) {
    char* end_ptr;
    unsigned long long param = 0;

    errno = 0;
    param = strtoull(arg, &end_ptr, 10);

    if (errno == ERANGE || errno == EINVAL || end_ptr == arg || *end_ptr != '\0') {
        return jp_errno_log_err_format(JP_EBUFFER_SIZE, "Size is invalid: '%.32s'.", arg);
    }

    if (param < JP_CONF_BUFFER_SIZE_MIN || param > JP_CONF_BUFFER_SIZE_MAX) {
        return jp_errno_log_err_format(JP_EBUFFER_SIZE, "Size is invalid: '%.32s'.", arg);
    }

    ctx->buffer_size = (size_t) param;
    return 0;
}

static jp_errno_t set_policy(const char* arg, worker_ctx_t* ctx) {
    if (!strcmp(arg, "wait")) {
        ctx->policy = JP_QUEUE_POLICY_WAIT;
        return 0;
    }

    if (!strcmp(arg, "drop")) {
        ctx->policy = JP_QUEUE_POLICY_DROP;
        return 0;
    }

    return jp_errno_log_err_format(JP_EOVERFLOW_POLICY, "Policy is invalid: '%.32s'.", arg);
}

static jp_errno_t handle_unknown_argument(const char* cmd) {
    return jp_errno_log_err_format(JP_EUNKNOWN_RUN_CMD, "Invalid or incomplete command: '%.32s'.", cmd);
}

static jp_errno_t create_and_normalize_out_dir(worker_ctx_t* ctx) {
    char tmp[JP_PATH_MAX]           = {0};
    char absolute_path[JP_PATH_MAX] = {0};
    char* p                         = NULL;
    struct stat st;

    if (ctx->out_dir == NULL) {
        JP_ERRNO_ALLOC(ctx->out_dir, strdup(JP_CONF_OUTDIR_DEF));
    }
    size_t path_len = strlen(ctx->out_dir);
    strncpy(tmp, ctx->out_dir, sizeof(tmp));

    while (path_len > 1 && tmp[path_len - 1] == '/') {
        tmp[path_len - 1] = '\0';
        path_len--;
    }

    for (p = tmp + 1; *p; p++) {
        if (*p == '/') {
            *p = '\0';
            if (mkdir(tmp, 0755) != 0 && errno != EEXIST) {
                return jp_errno_log_err_format(JP_EOUT_DIR, "Could not create the output directory: '%s'", tmp);
            }
            *p = '/';
        }
    }

    if (mkdir(tmp, 0755) != 0 && errno != EEXIST) {
        return jp_errno_log_err_format(JP_EOUT_DIR, "Could not create the directory: '%s'.", tmp);
    }

    if (stat(tmp, &st) != 0 || !S_ISDIR(st.st_mode)) {
        return jp_errno_log_err_format(JP_EOUT_DIR, "The target is not a directory: '%s'.", tmp);
    }

    if (access(tmp, W_OK) != 0) {
        return jp_errno_log_err_format(JP_EOUT_DIR, "The target is inaccessible: '%s'.", tmp);
    }

    if (realpath(tmp, absolute_path) == NULL) {
        return jp_errno_log_err_format(JP_EOUT_DIR, "Could not resolve absolute path: '%s'.", tmp);
    }

    JP_FREE(ctx->out_dir);
    JP_ERRNO_ALLOC(ctx->out_dir, strdup(absolute_path));
    return 0;
}

static jp_errno_t init_worker_args(int argc, char* argv[], worker_ctx_t* ctx) {
    uint8_t field_arg_count = jp_cmd_count(argc, argv, "-f", "--field");
    if (field_arg_count > JP_CONF_FIELDS_MAX) {
        return jp_errno_log_err_format(JP_ETOO_MANY_FIELD, "Too many fields specified: '%d'", field_arg_count);
    }
    JP_ERRNO_ALLOC(ctx->fields, jp_field_set_create(field_arg_count));
    return 0;
}

static jp_errno_t collect_cli_args(int argc, char* argv[], worker_ctx_t* ctx) {
    int option;
    opterr = 0;
    optind = 1;

    static struct option long_options[] = {{"chunk-size", required_argument, 0, 'c'},
                                           {"buffer-size", required_argument, 0, 'b'},
                                           {"policy", required_argument, 0, 'p'},
                                           {"field", required_argument, 0, 'f'},
                                           {"output", required_argument, 0, 'o'},
                                           {"help", no_argument, 0, 'h'},
                                           {"dry-run", no_argument, 0, 'n'},
                                           {"quiet", no_argument, 0, 'q'},
                                           {"no-color", no_argument, 0, 'C'},
                                           {0, 0, 0, 0}};

    while ((option = getopt_long(argc, argv, ":c:b:p:o:f:hnq", long_options, NULL)) != -1) {
        switch (option) {
            case 'c':
                JP_VERIFY(set_chunk_size(optarg, ctx));
                break;
            case 'b':
                JP_VERIFY(set_buffer_size(optarg, ctx));
                break;
            case 'p':
                JP_VERIFY(set_policy(optarg, ctx));
                break;
            case 'o':
                JP_VERIFY(set_out_dir(optarg, ctx));
                break;
            case 'n':
                ctx->dry_run = true;
                break;
            case 'f':
                JP_VERIFY(set_field(optarg, ctx));
                break;
            case 'q':
                JP_ATTR_FALLTHROUGH;
            case 'C':
                JP_ATTR_FALLTHROUGH;
            case 'h':
                break;
            case ':':
                JP_ATTR_FALLTHROUGH;
            case '?':
                JP_VERIFY(handle_unknown_argument((optind < argc) ? argv[optind] : argv[argc - 1]));
                break;
            default: {
            }
        }
    }

    if (optind < argc) {
        JP_VERIFY(handle_unknown_argument(argv[optind]));
    }

    return 0;
}

static jp_errno_t finalize_worker_args(worker_ctx_t* ctx) {
    JP_VERIFY(create_and_normalize_out_dir(ctx));
    JP_ERRNO_ALLOC(ctx->queue, jp_queue_create(ctx->buffer_size, ctx->chunk_size, ctx->policy));
    return 0;
}

static void* consumer_thread_init(void* data) {
    const worker_ctx_t* args   = data;
    jp_reader_ctx_t reader_ctx = {
        .chunk_size   = args->chunk_size,
        .queue        = args->queue,
        .input_stream = args->input_stream,
    };
    jp_errno_t err = jp_reader_consume(reader_ctx);
    pthread_exit((void*) (uintptr_t) err);  // NOLINT(performance-no-int-to-ptr)
}

static void* producer_thread_init(void* data) {
    const worker_ctx_t* args   = data;
    jp_writer_ctx_t writer_ctx = {
        .chunk_size = args->chunk_size,
        .queue      = args->queue,
        .output_dir = args->out_dir,
    };
    jp_errno_t err = jp_writer_produce(writer_ctx);
    pthread_exit((void*) (uintptr_t) err);  // NOLINT(performance-no-int-to-ptr)
}

static void* watcher_thread_init(void* data) {
    int sig;
    sigset_t set;
    worker_ctx_t* args = data;

    sigemptyset(&set);
    sigaddset(&set, SIGINT);
    sigaddset(&set, SIGTERM);
    if (sigwait(&set, &sig) == 0) {
        JP_LOG_DEBUG("[WATCHER]: Termination signal (%s) was received. Shutting down...",
                     sig == SIGINT ? "SIGINT" : "SIGTERM");
        jp_queue_finalize(args->queue);
    }
    pthread_exit(NULL);
}

static jp_errno_t orchestrate_threads(worker_ctx_t* ctx) {
    int err = 0, join_err = 0, t_size = 0;
    void* thread_result;
    sigset_t set;
    worker_thread_t threads[3] = {{.func = consumer_thread_init, .running = false, .detached = false},
                                  {.func = producer_thread_init, .running = false, .detached = false},
                                  {.func = watcher_thread_init, .running = false, .detached = true}};

    sigemptyset(&set);
    sigaddset(&set, SIGTERM);
    sigaddset(&set, SIGINT);
    pthread_sigmask(SIG_BLOCK, &set, NULL);

    t_size = sizeof(threads) / sizeof(threads[0]);
    for (int i = 0; i < t_size; i++) {
        err = pthread_create(&threads[i].tid, NULL, threads[i].func, ctx);
        if (err) {
            goto clean_up;
        }
        threads[i].running = true;
        if (!threads[i].detached) {
            continue;
        }
        err = pthread_detach(threads[i].tid);
        if (err) {
            goto clean_up;
        }
    }

clean_up:
    if (err) {
        jp_errno_log_err_format(JP_ERUN_FAILED, "%s", strerror(err));
    }

    for (int i = 0; i < t_size; i++) {
        if (threads[i].running && !threads[i].detached) {
            join_err = pthread_join(threads[i].tid, &thread_result);
            if (join_err) {
                jp_errno_log_err_format(JP_ERUN_FAILED, "%s", strerror(join_err));
            }
            const int thread_err = (int) (uintptr_t) thread_result;
            threads[i].running   = false;
            if (thread_err > 0) {
                err = thread_err;
            }
        }
    }

    jp_queue_finalize(ctx->queue);
    return err ? JP_ERUN_FAILED : 0;
}

jp_errno_t jp_wrk_exec(int argc, char* argv[]) {
    jp_errno_t err   = 0;
    worker_ctx_t ctx = {
        .input_stream = STDIN_FILENO,
        .buffer_size  = JP_CONF_BUFFER_SIZE_DEF,
        .chunk_size   = JP_CONF_CHUNK_SIZE_DEF,
        .out_dir      = NULL,
    };

    if (jp_cmd_count(argc, argv, "-h", "--help") > 0) {
        return display_help();
    }

    err = init_worker_args(argc, argv, &ctx);
    if (err) {
        goto clean_up;
    }

    err = collect_cli_args(argc, argv, &ctx);
    if (err) {
        goto clean_up;
    }

    err = finalize_worker_args(&ctx);
    if (err) {
        goto clean_up;
    }

    display_summary(&ctx);
    if (!ctx.dry_run) {
        err = orchestrate_threads(&ctx);
    }

clean_up:
    JP_FREE(ctx.out_dir);
    jp_field_set_destroy(ctx.fields);
    jp_queue_destroy(ctx.queue);
    return err;
}
