#include <stdio.h>
#include <assert.h>
#include <getopt.h>
#include <j_options.h>
#include <j_errno.h>

typedef struct {
    int argc;
    char *argv[10];
    int err;
    size_t buf_size;
    size_t que_size;
} test_case_t;

void test_buffer_size() {
    test_case_t cases[] = {
            {.argc = 1, .argv = {"jpipe",  NULL}, .err=0, .buf_size = J_BUF_DEF_SIZE},
            {.argc = 3, .argv = {"jpipe", "-b", "-1", NULL}, .err=J_EBUFFER_SIZE, .buf_size = J_BUF_DEF_SIZE},
            {.argc = 3, .argv = {"jpipe", "-b", "", NULL}, .err=J_EBUFFER_SIZE, .buf_size = J_BUF_DEF_SIZE},
            {.argc = 3, .argv = {"jpipe", "-b", "A", NULL}, .err=J_EBUFFER_SIZE, .buf_size = J_BUF_DEF_SIZE},
            {.argc = 3, .argv = {"jpipe", "-b", "0kb", NULL}, .err=J_EBUFFER_SIZE, .buf_size = J_BUF_DEF_SIZE},
            {.argc = 3, .argv = {"jpipe", "-b", "65537kb", NULL}, .err=J_EBUFFER_SIZE, .buf_size = J_BUF_DEF_SIZE},
            {.argc = 3, .argv = {"jpipe", "-b", "10Kb", NULL}, .err=J_EBUFFER_SIZE, .buf_size = J_BUF_DEF_SIZE},
            {.argc = 3, .argv = {"jpipe", "-b", "10kbx", NULL}, .err=J_EBUFFER_SIZE, .buf_size = J_BUF_DEF_SIZE},
            {.argc = 3, .argv = {"jpipe", "-b", "0mb", NULL}, .err=J_EBUFFER_SIZE, .buf_size = J_BUF_DEF_SIZE},
            {.argc = 3, .argv = {"jpipe", "-b", "65mb", NULL}, .err=J_EBUFFER_SIZE, .buf_size = J_BUF_DEF_SIZE},
            {.argc = 3, .argv = {"jpipe", "-b", "64Mb", NULL}, .err=J_EBUFFER_SIZE, .buf_size = J_BUF_DEF_SIZE},
            {.argc = 3, .argv = {"jpipe", "-b", "64mbx", NULL}, .err=J_EBUFFER_SIZE, .buf_size = J_BUF_DEF_SIZE},
            {.argc = 3, .argv = {"jpipe", "-b", "1kb", NULL}, .err=0, .buf_size = BYTES_IN_KB},
            {.argc = 3, .argv = {"jpipe", "-b", "10kb", NULL}, .err=0, .buf_size = 10 * BYTES_IN_KB},
            {.argc = 3, .argv = {"jpipe", "-b", "65536kb", NULL}, .err=0, .buf_size = 65536 * BYTES_IN_KB},
            {.argc = 3, .argv = {"jpipe", "-b", "1mb", NULL}, .err=0, .buf_size = 1 * BYTES_IN_MB},
            {.argc = 3, .argv = {"jpipe", "-b", "64mb", NULL}, .err=0, .buf_size = 64 * BYTES_IN_MB},
            {.argc = 3, .argv = {"jpipe", "--buffer", "-1", NULL}, .err=J_EBUFFER_SIZE, .buf_size = J_BUF_DEF_SIZE},
            {.argc = 3, .argv = {"jpipe", "--buffer", "", NULL}, .err=J_EBUFFER_SIZE, .buf_size = J_BUF_DEF_SIZE},
            {.argc = 3, .argv = {"jpipe", "--buffer", "A", NULL}, .err=J_EBUFFER_SIZE, .buf_size = J_BUF_DEF_SIZE},
            {.argc = 3, .argv = {"jpipe", "--buffer", "0kb", NULL}, .err=J_EBUFFER_SIZE, .buf_size = J_BUF_DEF_SIZE},
            {.argc = 3, .argv = {"jpipe", "--buffer", "65537kb", NULL}, .err=J_EBUFFER_SIZE, .buf_size = J_BUF_DEF_SIZE},
            {.argc = 3, .argv = {"jpipe", "--buffer", "10Kb", NULL}, .err=J_EBUFFER_SIZE, .buf_size = J_BUF_DEF_SIZE},
            {.argc = 3, .argv = {"jpipe", "--buffer", "10kbx", NULL}, .err=J_EBUFFER_SIZE, .buf_size = J_BUF_DEF_SIZE},
            {.argc = 3, .argv = {"jpipe", "--buffer", "0mb", NULL}, .err=J_EBUFFER_SIZE, .buf_size = J_BUF_DEF_SIZE},
            {.argc = 3, .argv = {"jpipe", "--buffer", "65mb", NULL}, .err=J_EBUFFER_SIZE, .buf_size = J_BUF_DEF_SIZE},
            {.argc = 3, .argv = {"jpipe", "--buffer", "64Mb", NULL}, .err=J_EBUFFER_SIZE, .buf_size = J_BUF_DEF_SIZE},
            {.argc = 3, .argv = {"jpipe", "--buffer", "64mbx", NULL}, .err=J_EBUFFER_SIZE, .buf_size = J_BUF_DEF_SIZE},
            {.argc = 3, .argv = {"jpipe", "--buffer", "1kb", NULL}, .err=0, .buf_size = BYTES_IN_KB},
            {.argc = 3, .argv = {"jpipe", "--buffer", "10kb", NULL}, .err=0, .buf_size = 10 * BYTES_IN_KB},
            {.argc = 3, .argv = {"jpipe", "--buffer", "65536kb", NULL}, .err=0, .buf_size = 65536 * BYTES_IN_KB},
            {.argc = 3, .argv = {"jpipe", "--buffer", "1mb", NULL}, .err=0, .buf_size = 1 * BYTES_IN_MB},
            {.argc = 3, .argv = {"jpipe", "--buffer", "64mb", NULL}, .err=0, .buf_size = 64 * BYTES_IN_MB},
    };

    int len = (sizeof (cases) / sizeof (cases[0]));
    for (int i = 0; i < len; i++) {
        int err = 0;
        jp_options_t opt = {};

        optind = 1;
        optarg = NULL;
        opterr = 0;

        err = jp_options_init(cases[i].argc, cases[i].argv, &opt);

        assert(err == cases[i].err);
        assert(opt.buf_size == cases[i].buf_size);
    }
}

void test_queue_size() {
    test_case_t cases[] = {
            {.argc = 1, .argv = {"jpipe",  NULL}, .err=0, .que_size = 0},
            {.argc = 3, .argv = {"jpipe", "-q", "-1", NULL}, .err=J_EQUEUE_LENGTH, .que_size = 0},
            {.argc = 3, .argv = {"jpipe", "-q", "", NULL}, .err=J_EQUEUE_LENGTH, .que_size = 0},
            {.argc = 3, .argv = {"jpipe", "-q", "A", NULL}, .err=J_EQUEUE_LENGTH, .que_size = 0},
            {.argc = 3, .argv = {"jpipe", "-q", "1025", NULL}, .err=J_EQUEUE_LENGTH, .que_size = 0},
            {.argc = 3, .argv = {"jpipe", "-q", "0", NULL}, .err=0, .que_size = 0},
            {.argc = 3, .argv = {"jpipe", "-q", "1", NULL}, .err=0, .que_size = 1},
            {.argc = 3, .argv = {"jpipe", "--queue", "1024", NULL}, .err=0, .que_size = 1024},
            {.argc = 3, .argv = {"jpipe", "--queue", "-1", NULL}, .err=J_EQUEUE_LENGTH, .que_size = 0},
            {.argc = 3, .argv = {"jpipe", "--queue", "", NULL}, .err=J_EQUEUE_LENGTH, .que_size = 0},
            {.argc = 3, .argv = {"jpipe", "--queue", "A", NULL}, .err=J_EQUEUE_LENGTH, .que_size = 0},
            {.argc = 3, .argv = {"jpipe", "--queue", "1025", NULL}, .err=J_EQUEUE_LENGTH, .que_size = 0},
            {.argc = 3, .argv = {"jpipe", "--queue", "0", NULL}, .err=0, .que_size = 0},
            {.argc = 3, .argv = {"jpipe", "--queue", "1", NULL}, .err=0, .que_size = 1},
            {.argc = 3, .argv = {"jpipe", "--queue", "1024", NULL}, .err=0, .que_size = 1024},
    };
    
    int len = (sizeof (cases) / sizeof (cases[0]));
    for (int i = 0; i < len; i++) {
        int err = 0;
        jp_options_t opt = {};

        optind = 1;
        optarg = NULL;
        opterr = 0;
        
        err = jp_options_init(cases[i].argc, cases[i].argv, &opt);

        assert(err == cases[i].err);
        assert(opt.que_size == cases[i].que_size);
    }
}

int main() {
    test_buffer_size();
    test_queue_size();
    return 0;
}