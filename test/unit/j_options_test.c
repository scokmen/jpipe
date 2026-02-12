#include <stdlib.h>
#include <assert.h>
#include <getopt.h>
#include <string.h>
#include <j_options.h>
#include <j_errno.h>

typedef struct {
    int argc;
    char *argv[10];
    int err;
    size_t chunk_size;
    size_t backlog_len;
    const char *out_dir;
} test_case_t;

void tear_down(jp_options_t *opts) {
    FREE_IF_ALLOC(opts->out_dir);
}

void test_chunk_size_opt() {
    test_case_t cases[] = {
            {.argc = 1, .argv = {"jpipe", NULL}, .err=0, .chunk_size = J_CHUNK_SIZE_DEF},
            {.argc = 3, .argv = {"jpipe", "-c", "-1", NULL}, .err=J_ECHUNK_SIZE, .chunk_size = J_CHUNK_SIZE_DEF},
            {.argc = 3, .argv = {"jpipe", "-c", "", NULL}, .err=J_ECHUNK_SIZE, .chunk_size = J_CHUNK_SIZE_DEF},
            {.argc = 3, .argv = {"jpipe", "-c", "A", NULL}, .err=J_ECHUNK_SIZE, .chunk_size = J_CHUNK_SIZE_DEF},
            {.argc = 3, .argv = {"jpipe", "-c", "0kb", NULL}, .err=J_ECHUNK_SIZE, .chunk_size = J_CHUNK_SIZE_DEF},
            {.argc = 3, .argv = {"jpipe", "-c", "65537kb", NULL}, .err=J_ECHUNK_SIZE, .chunk_size = J_CHUNK_SIZE_DEF},
            {.argc = 3, .argv = {"jpipe", "-c", "10Kb", NULL}, .err=J_ECHUNK_SIZE, .chunk_size = J_CHUNK_SIZE_DEF},
            {.argc = 3, .argv = {"jpipe", "-c", "10kbx", NULL}, .err=J_ECHUNK_SIZE, .chunk_size = J_CHUNK_SIZE_DEF},
            {.argc = 3, .argv = {"jpipe", "-c", "0mb", NULL}, .err=J_ECHUNK_SIZE, .chunk_size = J_CHUNK_SIZE_DEF},
            {.argc = 3, .argv = {"jpipe", "-c", "65mb", NULL}, .err=J_ECHUNK_SIZE, .chunk_size = J_CHUNK_SIZE_DEF},
            {.argc = 3, .argv = {"jpipe", "-c", "64Mb", NULL}, .err=J_ECHUNK_SIZE, .chunk_size = J_CHUNK_SIZE_DEF},
            {.argc = 3, .argv = {"jpipe", "-c", "64mbx", NULL}, .err=J_ECHUNK_SIZE, .chunk_size = J_CHUNK_SIZE_DEF},
            {.argc = 3, .argv = {"jpipe", "-c", "1kb", NULL}, .err=0, .chunk_size = BYTES_IN_KB},
            {.argc = 3, .argv = {"jpipe", "-c", "10kb", NULL}, .err=0, .chunk_size = 10 * BYTES_IN_KB},
            {.argc = 3, .argv = {"jpipe", "-c", "65536kb", NULL}, .err=0, .chunk_size = 65536 * BYTES_IN_KB},
            {.argc = 3, .argv = {"jpipe", "-c", "1mb", NULL}, .err=0, .chunk_size = 1 * BYTES_IN_MB},
            {.argc = 3, .argv = {"jpipe", "-c", "64mb", NULL}, .err=0, .chunk_size = 64 * BYTES_IN_MB},
            {.argc = 3, .argv = {"jpipe", "--chunk-size", "-1",
                                 NULL}, .err=J_ECHUNK_SIZE, .chunk_size = J_CHUNK_SIZE_DEF},
            {.argc = 3, .argv = {"jpipe", "--chunk-size", "",
                                 NULL}, .err=J_ECHUNK_SIZE, .chunk_size = J_CHUNK_SIZE_DEF},
            {.argc = 3, .argv = {"jpipe", "--chunk-size", "A",
                                 NULL}, .err=J_ECHUNK_SIZE, .chunk_size = J_CHUNK_SIZE_DEF},
            {.argc = 3, .argv = {"jpipe", "--chunk-size", "0kb",
                                 NULL}, .err=J_ECHUNK_SIZE, .chunk_size = J_CHUNK_SIZE_DEF},
            {.argc = 3, .argv = {"jpipe", "--chunk-size", "65537kb",
                                 NULL}, .err=J_ECHUNK_SIZE, .chunk_size = J_CHUNK_SIZE_DEF},
            {.argc = 3, .argv = {"jpipe", "--chunk-size", "10Kb",
                                 NULL}, .err=J_ECHUNK_SIZE, .chunk_size = J_CHUNK_SIZE_DEF},
            {.argc = 3, .argv = {"jpipe", "--chunk-size", "10kbx",
                                 NULL}, .err=J_ECHUNK_SIZE, .chunk_size = J_CHUNK_SIZE_DEF},
            {.argc = 3, .argv = {"jpipe", "--chunk-size", "0mb",
                                 NULL}, .err=J_ECHUNK_SIZE, .chunk_size = J_CHUNK_SIZE_DEF},
            {.argc = 3, .argv = {"jpipe", "--chunk-size", "65mb",
                                 NULL}, .err=J_ECHUNK_SIZE, .chunk_size = J_CHUNK_SIZE_DEF},
            {.argc = 3, .argv = {"jpipe", "--chunk-size", "64Mb",
                                 NULL}, .err=J_ECHUNK_SIZE, .chunk_size = J_CHUNK_SIZE_DEF},
            {.argc = 3, .argv = {"jpipe", "--chunk-size", "64mbx",
                                 NULL}, .err=J_ECHUNK_SIZE, .chunk_size = J_CHUNK_SIZE_DEF},
            {.argc = 3, .argv = {"jpipe", "--chunk-size", "1kb", NULL}, .err=0, .chunk_size = BYTES_IN_KB},
            {.argc = 3, .argv = {"jpipe", "--chunk-size", "10kb", NULL}, .err=0, .chunk_size = 10 * BYTES_IN_KB},
            {.argc = 3, .argv = {"jpipe", "--chunk-size", "65536kb", NULL}, .err=0, .chunk_size = 65536 * BYTES_IN_KB},
            {.argc = 3, .argv = {"jpipe", "--chunk-size", "1mb", NULL}, .err=0, .chunk_size = 1 * BYTES_IN_MB},
            {.argc = 3, .argv = {"jpipe", "--chunk-size", "64mb", NULL}, .err=0, .chunk_size = 64 * BYTES_IN_MB},
    };

    int len = (sizeof(cases) / sizeof(cases[0]));
    for (int i = 0; i < len; i++) {
        int err = 0;
        jp_options_t opt = {};

        optind = 1;
        optarg = NULL;
        opterr = 0;

        err = jp_options_init(cases[i].argc, cases[i].argv, &opt);

        assert(err == cases[i].err);
        assert(opt.chunk_size == cases[i].chunk_size);
        tear_down(&opt);
    }
}

void test_backlog_len() {
    test_case_t cases[] = {
            {.argc = 1, .argv = {"jpipe", NULL}, .err=0, .backlog_len = 0},
            {.argc = 3, .argv = {"jpipe", "-b", "-1", NULL}, .err=J_EBACKLOG_LENGTH, .backlog_len = 0},
            {.argc = 3, .argv = {"jpipe", "-b", "", NULL}, .err=J_EBACKLOG_LENGTH, .backlog_len = 0},
            {.argc = 3, .argv = {"jpipe", "-b", "A", NULL}, .err=J_EBACKLOG_LENGTH, .backlog_len = 0},
            {.argc = 3, .argv = {"jpipe", "-b", "1025", NULL}, .err=J_EBACKLOG_LENGTH, .backlog_len = 0},
            {.argc = 3, .argv = {"jpipe", "-b", "0", NULL}, .err=0, .backlog_len = 0},
            {.argc = 3, .argv = {"jpipe", "-b", "1", NULL}, .err=0, .backlog_len = 1},
            {.argc = 3, .argv = {"jpipe", "--backlog", "1024", NULL}, .err=0, .backlog_len = 1024},
            {.argc = 3, .argv = {"jpipe", "--backlog", "-1", NULL}, .err=J_EBACKLOG_LENGTH, .backlog_len = 0},
            {.argc = 3, .argv = {"jpipe", "--backlog", "", NULL}, .err=J_EBACKLOG_LENGTH, .backlog_len = 0},
            {.argc = 3, .argv = {"jpipe", "--backlog", "A", NULL}, .err=J_EBACKLOG_LENGTH, .backlog_len = 0},
            {.argc = 3, .argv = {"jpipe", "--backlog", "1025", NULL}, .err=J_EBACKLOG_LENGTH, .backlog_len = 0},
            {.argc = 3, .argv = {"jpipe", "--backlog", "0", NULL}, .err=0, .backlog_len = 0},
            {.argc = 3, .argv = {"jpipe", "--backlog", "1", NULL}, .err=0, .backlog_len = 1},
            {.argc = 3, .argv = {"jpipe", "--backlog", "1024", NULL}, .err=0, .backlog_len = 1024},
    };

    int len = (sizeof(cases) / sizeof(cases[0]));
    for (int i = 0; i < len; i++) {
        int err = 0;
        jp_options_t opt = {};

        optind = 1;
        optarg = NULL;
        opterr = 0;

        err = jp_options_init(cases[i].argc, cases[i].argv, &opt);

        assert(err == cases[i].err);
        assert(opt.backlog_len == cases[i].backlog_len);
        tear_down(&opt);
    }
}

void test_out_dir() {
    test_case_t cases[] = {
            {.argc = 1, .argv = {"jpipe", NULL}, .err=0, .out_dir = J_OUTDIR_DEF},
            {.argc = 3, .argv = {"jpipe", "-o", "", NULL}, .err= J_EOUT_DIR, .out_dir = NULL},
            {.argc = 3, .argv = {"jpipe", "-o", "/tmp", NULL}, .err= 0, .out_dir = "/tmp"},
            {.argc = 3, .argv = {"jpipe", "--out-dir", "", NULL}, .err= J_EOUT_DIR, .out_dir = NULL},
            {.argc = 3, .argv = {"jpipe", "--out-dir", "/tmp", NULL}, .err= 0, .out_dir = "/tmp"},
    };

    int len = (sizeof(cases) / sizeof(cases[0]));
    for (int i = 0; i < len; i++) {
        int err = 0;
        jp_options_t opt = {};

        optind = 1;
        optarg = NULL;
        opterr = 0;

        err = jp_options_init(cases[i].argc, cases[i].argv, &opt);

        assert(err == cases[i].err);
        if (err) {
            assert(opt.out_dir == NULL);
        } else {
            assert(!strcmp(opt.out_dir, cases[i].out_dir));
        }
        tear_down(&opt);
    }
}

int main() {
    test_chunk_size_opt();
    test_backlog_len();
    test_out_dir();
    return 0;
}