#include <getopt.h>
#include <jp_command.h>
#include <jp_encoder.h>
#include <jp_errno.h>
#include <jp_field.h>
#include <jp_memory.h>
#include <jp_queue.h>
#include <jp_reader.h>
#include <jp_worker.h>
#include <jp_writer.h>
#include <limits.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

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
    JP_LOG_INFO("    - key=1e+10      -> Number  (no quotes in JSON).");
    JP_LOG_INFO("    - key=true|false -> Boolean (no quotes in JSON).");
    JP_LOG_INFO("    - key=string     -> String.");
    JP_LOG_INFO("    - key=\"123\"      -> Forced string.");
    JP_LOG_INFO("    - key=\"true\"     -> Forced string.");
    JP_LOG_INFO("\n  Example:");
    JP_LOG_INFO("    Input : jpipe -f \"id=101\" -f \"name=app\" -f \"active=true\" -f \"ver=1.2.0\"");
    JP_LOG_INFO("    Output: {\"id\": 101, \"name\": \"app\", \"active\": true, \"ver\": \"1.2.0\"}");
    return 0;
}

static void display_summary(worker_ctx_t* ctx) {
    const double estimated_mem_usage =
        (double) ctx->chunk_size * (double) ctx->buffer_size / (BYTES_IN_KB * BYTES_IN_KB);

    JP_LOG_MSG("Application is starting...\n");
    JP_LOG_MSG("[Runtime Parameters]");
    JP_LOG_MSG("• Chunk Size   (-c) : %zu KB", (ctx->chunk_size / BYTES_IN_KB));
    JP_LOG_MSG("• Buffer Size  (-b) : %zu", ctx->buffer_size);
    JP_LOG_MSG("• Output Dir   (-o) : %s", ctx->out_dir);
    JP_LOG_MSG("• Policy       (-p) : %s", ctx->policy == JP_QUEUE_POLICY_WAIT ? "WAIT" : "DROP");
    if (ctx->fields->len > 0) {
        JP_LOG_MSG("• Fields       (-f) :");
        for (size_t i = 0; i < ctx->fields->len; i++) {
            JP_LOG_MSG("     %zu. %-32.*s= %-32.*s",
                       i + 1,
                       (int) ctx->fields->fields[i]->key_len,
                       ctx->fields->fields[i]->key,
                       (int) ctx->fields->fields[i]->val_len,
                       (char*) ctx->fields->fields[i]->val);
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
        return JP_ERRNO_RAISE(JP_EOUT_DIR);
    }

    len = strlen(arg);
    if (len == 0) {
        return JP_ERRNO_RAISE(JP_EOUT_DIR);
    }
    if (len > JP_PATH_MAX) {
        return JP_ERRNO_RAISEF(JP_EOUT_DIR, "Path is too long. Maximum allowed length: %d", JP_PATH_MAX);
    }

    ctx->out_dir = jp_mem_strdup(arg);
    return 0;
}

static jp_errno_t set_field(const char* arg, worker_ctx_t* ctx) {
    if (arg == NULL) {
        return JP_ERRNO_RAISE(JP_EINV_FIELD_KEY);
    }

    JP_ATTR_ASSUME(ctx->fields != NULL);

    const jp_errno_t err = jp_field_set_add(ctx->fields, arg);
    if (err == 0) {
        return 0;
    }
    return err == JP_ETOO_MANY_FIELD ? JP_ERRNO_RAISE(err) : JP_ERRNO_RAISEF(err, "Field is invalid: \"%s\"", arg);
}

static jp_errno_t set_chunk_size(const char* arg, worker_ctx_t* ctx) {
    char* end_ptr;
    size_t chunk_size        = 0;
    unsigned long long param = 0;

    if (arg == NULL) {
        return JP_ERRNO_RAISE(JP_ECHUNK_SIZE);
    }

    errno = 0;
    param = strtoull(arg, &end_ptr, 10);

    if (errno == ERANGE || errno == EINVAL) {
        return JP_ERRNO_RAISEF(JP_ECHUNK_SIZE, "Size is invalid: \"%.32s\"", arg);
    }

    if (end_ptr == arg) {
        return JP_ERRNO_RAISEF(JP_ECHUNK_SIZE, "Size is invalid: \"%.32s\"", arg);
    }

    if (*end_ptr != '\0' && !strcmp(end_ptr, "kb") && param <= JP_CONF_CHUNK_SIZE_MAX / BYTES_IN_KB) {
        chunk_size += param * BYTES_IN_KB;
    } else {
        return JP_ERRNO_RAISEF(JP_ECHUNK_SIZE, "Size is invalid: \"%.32s\"", arg);
    }

    if (chunk_size < JP_CONF_CHUNK_SIZE_MIN || chunk_size > JP_CONF_CHUNK_SIZE_MAX) {
        return JP_ERRNO_RAISEF(JP_ECHUNK_SIZE, "Size is invalid: \"%.32s\"", arg);
    }
    ctx->chunk_size = chunk_size;
    return 0;
}

static jp_errno_t set_buffer_size(const char* arg, worker_ctx_t* ctx) {
    char* end_ptr;
    unsigned long long param = 0;

    if (arg == NULL) {
        return JP_ERRNO_RAISE(JP_EBUFFER_SIZE);
    }

    errno = 0;
    param = strtoull(arg, &end_ptr, 10);

    if (errno == ERANGE || errno == EINVAL || end_ptr == arg || *end_ptr != '\0') {
        return JP_ERRNO_RAISEF(JP_EBUFFER_SIZE, "Size is invalid: \"%.32s\"", arg);
    }

    if (param < JP_CONF_BUFFER_SIZE_MIN || param > JP_CONF_BUFFER_SIZE_MAX) {
        return JP_ERRNO_RAISEF(JP_EBUFFER_SIZE, "Size is invalid: \"%.32s\"", arg);
    }

    ctx->buffer_size = (size_t) param;
    return 0;
}

static jp_errno_t set_policy(const char* arg, worker_ctx_t* ctx) {
    if (arg == NULL) {
        return JP_ERRNO_RAISE(JP_EOVERFLOW_POLICY);
    }

    if (!strcmp(arg, "wait")) {
        ctx->policy = JP_QUEUE_POLICY_WAIT;
        return 0;
    }
    if (!strcmp(arg, "drop")) {
        ctx->policy = JP_QUEUE_POLICY_DROP;
        return 0;
    }

    return JP_ERRNO_RAISEF(JP_EOVERFLOW_POLICY, "Policy is invalid: \"%.32s\"", arg);
}

static jp_errno_t handle_unknown_argument(const char* cmd) {
    return JP_ERRNO_RAISEF(JP_EUNKNOWN_RUN_CMD, "Invalid or incomplete command: \"%.32s\"", cmd);
}

static jp_errno_t create_and_normalize_out_dir(worker_ctx_t* ctx) {
    char tmp[JP_PATH_MAX]           = {0};
    char absolute_path[JP_PATH_MAX] = {0};
    char* p                         = NULL;
    struct stat st;
    const char* output = ctx->out_dir != NULL ? ctx->out_dir : JP_CONF_OUTDIR_DEF;

    size_t path_len = strlen(output);
    strncpy(tmp, output, sizeof(tmp));

    while (path_len > 1 && tmp[path_len - 1] == '/') {
        tmp[path_len - 1] = '\0';
        path_len--;
    }

    for (p = tmp + 1; *p; p++) {
        if (*p == '/') {
            *p = '\0';
            if (mkdir(tmp, 0755) != 0 && errno != EEXIST) {
                return JP_ERRNO_RAISEF(JP_EOUT_DIR, "Could not create the output directory: \"%128s\"", tmp);
            }
            *p = '/';
        }
    }

    if (mkdir(tmp, 0755) != 0 && errno != EEXIST) {
        return JP_ERRNO_RAISEF(JP_EOUT_DIR, "Could not create the directory: \"%128s\"", tmp);
    }

    if (stat(tmp, &st) != 0 || !S_ISDIR(st.st_mode)) {
        return JP_ERRNO_RAISEF(JP_EOUT_DIR, "The target is not a directory: \"%128s\"", tmp);
    }

    if (access(tmp, W_OK) != 0) {
        return JP_ERRNO_RAISEF(JP_EOUT_DIR, "The target is inaccessible: \"%128s\"", tmp);
    }

    if (realpath(tmp, absolute_path) == NULL) {
        return JP_ERRNO_RAISEF(JP_EOUT_DIR, "Could not resolve absolute path: \"%128s\"", tmp);
    }

    JP_FREE(ctx->out_dir);
    ctx->out_dir = jp_mem_strdup(absolute_path);
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

    while ((option = getopt_long(argc, argv, ":c:b:p:o:f:hnqC", long_options, NULL)) != -1) {
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
    jp_errno_t err = 0;
    JP_VERIFY(create_and_normalize_out_dir(ctx));
    ctx->queue = jp_queue_create(ctx->buffer_size, ctx->chunk_size, ctx->policy, &err);
    if (err || ctx->queue == NULL) {
        return JP_ERRNO_RAISE(err ? err : JP_ESYS_ERR);
    }
    return 0;
}

static void thread_cleanup(void* data) {
    const worker_ctx_t* args = data;
    jp_queue_finalize(args->queue);
    JP_ERRNO_DUMP();
    JP_ERRNO_RESET();
}

static void* consumer_thread_init(void* data) {
    const worker_ctx_t* args         = data;
    const jp_reader_ctx_t reader_ctx = {
        .chunk_size   = args->chunk_size,
        .queue        = args->queue,
        .input_stream = args->input_stream,
    };

    pthread_cleanup_push(thread_cleanup, data);

    jp_errno_t* result = jp_mem_malloc(sizeof(jp_errno_t));
    *result            = jp_reader_consume(reader_ctx);
    pthread_exit(result);

    pthread_cleanup_pop(1);
}

static void* producer_thread_init(void* data) {
    const worker_ctx_t* args         = data;
    const jp_writer_ctx_t writer_ctx = {.chunk_size = args->chunk_size,
                                        .queue      = args->queue,
                                        .output_dir = args->out_dir,
                                        .fields     = args->fields,
                                        .encoder    = jp_encoder_json};

    pthread_cleanup_push(thread_cleanup, data);

    jp_errno_t* result = jp_mem_malloc(sizeof(jp_errno_t));
    *result            = jp_writer_produce(writer_ctx);
    pthread_exit(result);

    pthread_cleanup_pop(1);
}

static void* watcher_thread_init(void* data) {
    int sig;
    sigset_t set;
    const worker_ctx_t* args = data;

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

static jp_errno_t join_worker_thread(const pthread_t thread_id) {
    void* thread_result;
    const int err = pthread_join(thread_id, &thread_result);
    if (err) {
        return JP_ERRNO_RAISE_POSIX(JP_ESYS_ERR, err);
    }
    if (thread_result == NULL || thread_result == PTHREAD_CANCELED) {
        return 0;
    }
    const jp_errno_t thread_value = *(jp_errno_t*) thread_result;
    JP_FREE(thread_result);
    return thread_value;
}

static void cancel_worker_thread(const pthread_t thread_id) {
    const int err = pthread_cancel(thread_id);
    if (err) {
        JP_ERRNO_RAISE_POSIX(JP_ESYS_ERR, err);
    }
}

static jp_errno_t orchestrate_threads(worker_ctx_t* ctx) {
    int err = 0;
    sigset_t set;
    pthread_attr_t attr;
    struct sched_param param;
    pthread_t consumer_thread, producer_thread, watcher_thread;
    uint8_t flags = 0x0;

    sigemptyset(&set);
    sigaddset(&set, SIGTERM);
    sigaddset(&set, SIGINT);
    pthread_sigmask(SIG_BLOCK, &set, NULL);

    pthread_attr_init(&attr);

    err = pthread_create(&consumer_thread, &attr, consumer_thread_init, ctx);
    if (err) {
        goto clean_up;
    }
    flags = flags | 0x1;

    err = pthread_create(&producer_thread, &attr, producer_thread_init, ctx);
    if (err) {
        goto clean_up;
    }
    flags = flags | 0x2;

    param.sched_priority = 0;
    pthread_attr_setstacksize(&attr, PTHREAD_STACK_MIN);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthread_attr_setschedpolicy(&attr, SCHED_OTHER);
    pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
    pthread_attr_setschedparam(&attr, &param);

    err = pthread_create(&watcher_thread, &attr, watcher_thread_init, ctx);

clean_up:
    if (err) {
        JP_ERRNO_RAISE_POSIX(JP_ESYS_ERR, err);
        if (flags & 0x01) {
            cancel_worker_thread(consumer_thread);
        }
        if (flags & 0x02) {
            cancel_worker_thread(producer_thread);
        }
    }
    if (flags & 0x1) {
        err += (int) join_worker_thread(consumer_thread);
    }
    if (flags & 0x2) {
        err += (int) join_worker_thread(producer_thread);
    }

    jp_queue_finalize(ctx->queue);
    pthread_attr_destroy(&attr);
    return err ? JP_ERUN_FAILED : 0;
}

jp_errno_t jp_wrk_exec(int argc, char* argv[]) {
    jp_errno_t err   = 0;
    worker_ctx_t ctx = {.input_stream = STDIN_FILENO,
                        .buffer_size  = JP_CONF_BUFFER_SIZE_DEF,
                        .chunk_size   = JP_CONF_CHUNK_SIZE_DEF,
                        .out_dir      = NULL};

    if (jp_cmd_count(argc, argv, "-h", "--help") > 0) {
        return display_help();
    }

    ctx.fields = jp_field_set_create(JP_CONF_FIELDS_MAX);
    err        = collect_cli_args(argc, argv, &ctx);
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
