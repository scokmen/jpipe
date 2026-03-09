#include <errno.h>
#include <getopt.h>
#include <jp_command.h>
#include <jp_errno.h>
#include <jp_poller.h>
#include <jp_queue.h>
#include <jp_worker.h>
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
    size_t chunk_size;
    size_t buffer_size;
    bool dry_run;
    jp_queue_policy_t policy;
    const char* out_dir;
    jp_queue_t* queue;
    jp_field_set_t* fields;
} worker_arg_t;

static jp_errno_t display_help(void) {
    JP_LOG("Usage: jpipe run [options]\n");
    JP_LOG("Execute the data processing engine with the following configurations:\n");
    JP_LOG("Options:");
    JP_LOG("  -c, --chunk-size  <size>     Chunk size (e.g., 16kb, 64kb). Range: 1kb-128kb  (default: 16kb).");
    JP_LOG("  -b, --buffer-size <count>    Max pending operations. Range: 1-1024 (default: 64).");
    JP_LOG("  -p, --policy      <type>     Overflow policy: 'wait' or 'drop' (default: wait).");
    JP_LOG("  -o, --out-dir     <path>     Output directory (default: current dir).");
    JP_LOG("  -f, --field       key=value  Additional field to the JSON output. Can be used multiple times.");
    JP_LOG("  -n, --dry-run                Dry run.");
    JP_LOG("  -h, --help                   Show this help message.\n");
    JP_LOG("Field Options:");
    JP_LOG("  -f, --field \"key=value\"   Add a field to the JSON output.\n");
    JP_LOG("  Key Rules:");
    JP_LOG("    - Must contain only: 'a-z', 'A-Z', '0-9', '_' and '-'.");
    JP_LOG("    - Maximum length: 64 characters.\n");
    JP_LOG("  Value Type Inference:");
    JP_LOG("    - key=123        -> Number  (no quotes in JSON).");
    JP_LOG("    - key=true|false -> Boolean (no quotes in JSON).");
    JP_LOG("    - key=string     -> String.");
    JP_LOG("    - key=\"string\"   -> String.");
    JP_LOG("    - key=\"123\"      -> Forced string.");
    JP_LOG("    - key=\"true\"     -> Forced string.");
    JP_LOG("    - key=\"str=ng\"   -> String.");
    JP_LOG("    - key=str=ng     -> Invalid field.\n");
    JP_LOG("  Example:");
    JP_LOG("    Input : jpipe -f \"id=101\" -f \"name=app\" -f \"active=true\" -f \"ver=1.2.0\"");
    JP_LOG("    Output: {\"id\": 101, \"name\": \"app\", \"active\": true, \"ver\": \"1.2.0\"}");
    return 0;
}

static void display_summary(worker_arg_t* args) {
    double estimated_mem_usage = ((double) args->chunk_size * (double) args->buffer_size) / (BYTES_IN_KB * BYTES_IN_KB);

    JP_LOG("[jpipe]: Configuration Summary\n");
    JP_LOG("[Runtime Parameters]");
    JP_LOG("• Chunk Size   (-c) : %zu KB", (args->chunk_size / BYTES_IN_KB));
    JP_LOG("• Buffer Size  (-b) : %zu", args->buffer_size);
    JP_LOG("• Output Dir   (-o) : %s", args->out_dir);
    JP_LOG("• Policy       (-p) : %s", args->policy == JP_QUEUE_POLICY_WAIT ? "WAIT" : "DROP");
    if (args->fields->len > 0) {
        JP_LOG("• Fields       (-f) :");
        for (size_t i = 0; i < args->fields->len; i++) {
            JP_LOG("     %zu. %-32s= %-32s", i + 1, args->fields->fields[i]->key, args->fields->fields[i]->val);
        }
    }
    JP_LOG("\n[Resource Utilization]");
    JP_LOG("• Memory Usage      :  ~%.2f MB", estimated_mem_usage);
    JP_LOG("\nThese values are based on user-provided parameters.");
    JP_LOG("Memory usage is an approximation; operating system overhead and thread stack allocations are not included.");
}

static jp_errno_t set_out_dir(const char* arg, worker_arg_t* args) {
    size_t len = 0;
    JP_FREE(args->out_dir);
    if (arg == NULL) {
        return jp_errno_log_err_format(JP_EMISSING_CMD, "Output path is empty.");
    }
    len = strlen(arg);
    if (len == 0) {
        return jp_errno_log_err_format(JP_EMISSING_CMD, "Output path is empty.");
    }
    if (len > JP_PATH_MAX) {
        return jp_errno_log_err_format(JP_EMISSING_CMD, "Output path is too long. Maximum allowed path size: %d.", JP_PATH_MAX);
    }
    JP_ALLOC_OR_LOG(args->out_dir, strdup(arg));
    return 0;
}

static jp_errno_t set_field(const char* arg, worker_arg_t* args) {
    jp_errno_t err;
    if (arg == NULL) {
        return jp_errno_log_err_format(JP_EINV_FIELD_KEY, "Field key/value is empty");
    }

    JP_ASSUME(args->fields != NULL);
    err = jp_field_set_add(args->fields, arg);
    if (err) {
        return jp_errno_log_err_format(err, "Field key/value is invalid: '%s'", arg);
    }
    return 0;
}

static jp_errno_t set_chunk_size(const char* arg, worker_arg_t* args) {
    char* end_ptr;
    size_t chunk_size        = 0;
    unsigned long long param = 0;

    errno = 0;
    param = strtoull(arg, &end_ptr, 10);

    if (errno == ERANGE) {
        return jp_errno_log_err_format(JP_ECHUNK_SIZE, "Chunk size format is incorrect: '%.32s'.", arg);
    }

    if (end_ptr == arg) {
        return jp_errno_log_err_format(JP_ECHUNK_SIZE, "Chunk size is empty: '%.32s'.", arg);
    }

    if (*end_ptr != '\0' && !strcmp(end_ptr, "kb") && param <= (JP_WRK_CHUNK_SIZE_MAX / BYTES_IN_KB)) {
        chunk_size += (param * BYTES_IN_KB);
    } else {
        return jp_errno_log_err_format(JP_ECHUNK_SIZE, "Chunk size value is invalid: '%.32s'.", arg);
    }

    if (chunk_size < JP_WRK_CHUNK_SIZE_MIN || chunk_size > JP_WRK_CHUNK_SIZE_MAX) {
        return jp_errno_log_err_format(JP_ECHUNK_SIZE, "Chunk size value is invalid: '%.32s'.", arg);
    }
    args->chunk_size = (size_t) chunk_size;
    return 0;
}

static jp_errno_t set_buffer_size(const char* arg, worker_arg_t* args) {
    char* end_ptr;
    unsigned long long param = 0;

    errno = 0;
    param = strtoull(arg, &end_ptr, 10);

    if (errno == ERANGE || end_ptr == arg || *end_ptr != '\0') {
        return jp_errno_log_err_format(JP_EBUFFER_SIZE, "Buffer size format is incorrect: '%.32s'.", arg);
    }

    if (param < JP_WRK_BUFFER_SIZE_MIN || param > JP_WRK_BUFFER_SIZE_MAX) {
        return jp_errno_log_err_format(JP_EBUFFER_SIZE, "Buffer size format invalid: '%.32s'.", arg);
    }

    args->buffer_size = (size_t) param;
    return 0;
}

static jp_errno_t set_policy(const char* arg, worker_arg_t* args) {
    if (!strcmp(arg, "wait")) {
        args->policy = JP_QUEUE_POLICY_WAIT;
        return 0;
    }

    if (!strcmp(arg, "drop")) {
        args->policy = JP_QUEUE_POLICY_DROP;
        return 0;
    }

    return jp_errno_log_err_format(JP_EOVERFLOW_POLICY, "Overflow policy is invalid: '%.32s'.", arg);
}

static jp_errno_t handle_unknown_argument(const char* cmd) {
    return jp_errno_log_err_format(JP_EUNKNOWN_RUN_CMD, "Invalid or incomplete [run] command: '%.32s'.", cmd);
}

static jp_errno_t create_and_normalize_out_dir(worker_arg_t* args) {
    char tmp[JP_PATH_MAX]           = {0};
    char absolute_path[JP_PATH_MAX] = {0};
    char* p                         = NULL;
    struct stat st;

    if (args->out_dir == NULL) {
        JP_ALLOC_OR_LOG(args->out_dir, strdup(JP_WRK_OUTDIR_DEF));
    }
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
                return jp_errno_log_err_format(JP_EOUT_DIR, "Could not create the output directory: '%s'", tmp);
            }
            *p = '/';
        }
    }

    if (mkdir(tmp, 0755) != 0 && errno != EEXIST) {
        return jp_errno_log_err_format(JP_EOUT_DIR, "Could not create the output directory: '%s'.", tmp);
    }

    if (stat(tmp, &st) != 0 || !S_ISDIR(st.st_mode)) {
        return jp_errno_log_err_format(JP_EOUT_DIR, "The target path is not a directory: '%s'.", tmp);
    }

    if (access(tmp, W_OK) != 0) {
        return jp_errno_log_err_format(JP_EOUT_DIR, "The target path is inaccessible: '%s'.", tmp);
    }

    if (realpath(tmp, absolute_path) == NULL) {
        return jp_errno_log_err_format(JP_EOUT_DIR, "Could not resolve absolute path: '%s'.", tmp);
    }

    JP_FREE(args->out_dir);
    JP_ALLOC_OR_LOG(args->out_dir, strdup(absolute_path));
    return 0;
}

static int get_field_args_count(int argc, char* argv[]) {
    int c = 0;
    for (int i = 0; i < argc; i++) {
        if (JP_CMD_EQ(argv[i], "-f", "--field")) {
            c++;
        }
    }
    return c;
}

static jp_errno_t init_worker_args(int argc, char* argv[], worker_arg_t* args) {
    int fields = get_field_args_count(argc, argv);
    if (fields > JP_WRK_FIELDS_MAX) {
        return jp_errno_log_err_format(JP_ETOO_MANY_FIELD, "Too many 'fields' specified: '%d'", fields);
    }
    JP_ALLOC_OR_LOG(args->fields, jp_field_set_create((size_t) fields));
    return 0;
}

static jp_errno_t collect_cli_args(int argc, char* argv[], worker_arg_t* args) {
    int option;
    opterr            = 0;
    optind            = 1;
    args->chunk_size  = JP_WRK_CHUNK_SIZE_DEF;
    args->buffer_size = JP_WRK_BUFFER_SIZE_DEF;
    args->out_dir     = NULL;

    static struct option long_options[] = {{"chunk-size", required_argument, 0, 'c'},
                                           {"buffer-size", required_argument, 0, 'b'},
                                           {"policy", required_argument, 0, 'p'},
                                           {"field", required_argument, 0, 'f'},
                                           {"out-dir", required_argument, 0, 'o'},
                                           {"dry-run", required_argument, 0, 'n'},
                                           {"help", no_argument, 0, 'h'},
                                           {0, 0, 0, 0}};

    while ((option = getopt_long(argc, argv, ":c:b:p:o:f:hn", long_options, NULL)) != -1) {
        switch (option) {
            case 'c':
                JP_OK_OR_RET(set_chunk_size(optarg, args));
                break;
            case 'b':
                JP_OK_OR_RET(set_buffer_size(optarg, args));
                break;
            case 'p':
                JP_OK_OR_RET(set_policy(optarg, args));
                break;
            case 'o':
                JP_OK_OR_RET(set_out_dir(optarg, args));
                break;
            case 'n':
                args->dry_run = true;
                break;
            case 'f':
                JP_OK_OR_RET(set_field(optarg, args));
                break;
            case 'h':
                break;
            case ':':
                JP_FALLTHROUGH;
            case '?':
                JP_OK_OR_RET(handle_unknown_argument(argv[optind - 1]));
                break;
            default: {
            }
        }
    }

    if (optind < argc) {
        JP_OK_OR_RET(handle_unknown_argument(argv[optind]));
    }
    
    return 0;
}

static jp_errno_t finalize_worker_args(worker_arg_t* args) {
    JP_OK_OR_RET(create_and_normalize_out_dir(args));
    JP_ALLOC_OR_LOG(args->queue, jp_queue_create(args->buffer_size, args->chunk_size, args->policy));
    return 0;
}

static void* producer_thread_init(void* data) {
    size_t chunk_size;
    jp_errno_t err;
    unsigned char* buffer;
    jp_poller_t* poller;
    worker_arg_t* args = (worker_arg_t*) data;

    chunk_size = args->chunk_size;
    args       = (worker_arg_t*) data;
    buffer     = malloc(chunk_size);
    poller     = jp_poller_create(100);
    if (buffer == NULL || poller == NULL) {
        err = JP_ENOMEMORY;
        jp_errno_log_err(err);
        goto clean_up;
    }

    err = jp_poller_poll(poller, STDIN_FILENO);
    if (err) {
        jp_errno_log_err(err);
        goto clean_up;
    }

    while (true) {
        err = jp_poller_wait(poller);
        if (err == JP_EREAD_FAILED) {
            jp_errno_log_err(err);
            break;
        }
        if (err == JP_ETRYAGAIN) {
            continue;
        }
        while (true) {
            err       = 0;
            ssize_t n = read(STDIN_FILENO, buffer, chunk_size);
            if (n == 0) {
                jp_queue_finalize(args->queue);
                goto clean_up;
            }
            if (n < 0) {
                if (errno == EINTR) {
                    continue;
                }
                if (JP_IS_EAGAIN(errno)) {
                    break;
                }
                err = JP_EREAD_FAILED;
                jp_errno_log_err(err);
                jp_queue_finalize(args->queue);
                goto clean_up;
            }
            err = jp_queue_push(args->queue, buffer, (size_t) n);
            if (err == JP_EMSG_DROPPED) {
                // TODO: Handle message dropped.
                break;
            }
            if (err == JP_ESHUTTING_DOWN) {
                goto clean_up;
            }
        }
    }

clean_up:
    JP_FREE(buffer);
    jp_poller_destroy(poller);
    // TODO: Solve performance-no-int-to-ptr
    pthread_exit((void*) (uintptr_t) err);  // NOLINT(performance-no-int-to-ptr)
}

static void* consumer_thread_init(void* data) {
    jp_errno_t err;
    size_t max_len, read_len;
    unsigned char buffer[1024 * 16];
    worker_arg_t* args = (worker_arg_t*) data;

    max_len = args->chunk_size;
    while (true) {
        // TODO: Replace with the real implementation.
        err = jp_queue_pop(args->queue, buffer, max_len, &read_len);
        if (err) {
            JP_DEBUG("[CONSUMER]: Cannot pop.");
            break;
        }
    }

    // TODO: Solve performance-no-int-to-ptr
    pthread_exit((void*) (uintptr_t) err);  // NOLINT(performance-no-int-to-ptr)
}

static void* watcher_thread_init(void* data) {
    int sig;
    sigset_t set;
    worker_arg_t* args = (worker_arg_t*) data;

    sigemptyset(&set);
    sigaddset(&set, SIGINT);
    sigaddset(&set, SIGTERM);
    if (sigwait(&set, &sig) == 0) {
        JP_DEBUG("[WATCHER]: Termination signal (%s) was received. Shutting down...", sig == SIGINT ? "SIGINT" : "SIGTERM");
        jp_queue_finalize(args->queue);
    }
    pthread_exit(NULL);
}

static jp_errno_t orchestrate_threads(worker_arg_t* args) {
    int err = 0, join_err = 0, t_size = 0;
    void* thread_result;
    sigset_t set;
    worker_thread_t threads[3] = {{.func = producer_thread_init, .running = false, .detached = false},
                                  {.func = consumer_thread_init, .running = false, .detached = false},
                                  {.func = watcher_thread_init, .running = false, .detached = true}};

    sigemptyset(&set);
    sigaddset(&set, SIGTERM);
    sigaddset(&set, SIGINT);
    pthread_sigmask(SIG_BLOCK, &set, NULL);

    t_size = (sizeof(threads) / sizeof(threads[0]));
    for (int i = 0; i < t_size; i++) {
        err = pthread_create(&threads[i].tid, NULL, threads[i].func, args);
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
            err                = (int) (uintptr_t) thread_result;
            threads[i].running = false;
        }
    }

    return err ? JP_ERUN_FAILED : 0;
}

jp_errno_t jp_wrk_exec(int argc, char* argv[]) {
    jp_errno_t err    = 0;
    worker_arg_t args = {0};

    if (argc == 2 && JP_CMD_EQ(argv[1], "-h", "--help")) {
        return display_help();
    }

    err = init_worker_args(argc, argv, &args);
    if (err) {
        goto clean_up;
    }

    err = collect_cli_args(argc, argv, &args);
    if (err) {
        goto clean_up;
    }

    err = finalize_worker_args(&args);
    if (err) {
        goto clean_up;
    }

    display_summary(&args);
    if (!args.dry_run) {
        err = orchestrate_threads(&args);
    }

clean_up:
    JP_FREE(args.out_dir);
    jp_field_set_destroy(args.fields);
    jp_queue_destroy(args.queue);
    return err;
}
