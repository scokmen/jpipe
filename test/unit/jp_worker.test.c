#include <stdlib.h>
#include <stdio.h>
#include <jp_worker.h>
#include <jp_test.h>
#include <jp_errno.h>
#include <getopt.h>

typedef struct {
    int argc;
    int expected;
    char *argv[10];
    const char *expected_out_dir;
} test_case_t;

typedef struct {
    int argc;
    char *argv[4];
} test_ctx_t;

int cmd_work_exec_adapter(void *ctx) {
    test_ctx_t *c = (test_ctx_t *) ctx;
    return jp_wrk_exec(c->argc, c->argv);
}

void test_jp_wrk_exec_help_short() {
    test_ctx_t ctx = {.argc = 2, .argv = {"run", "-h", NULL}};
    int status = jp_test_compare_stdout(cmd_work_exec_adapter, &ctx, "run_command_help_option.tmpl");
    JP_ASSERT_EQ(0, status);
}

void test_jp_wrk_exec_help_long() {
    test_ctx_t ctx = {.argc = 2, .argv = {"run", "--help", NULL}};
    int status = jp_test_compare_stdout(cmd_work_exec_adapter, &ctx, "run_command_help_option.tmpl");
    JP_ASSERT_EQ(0, status);
}

void test_jp_wrk_exec_args_backlog_length() {
    test_case_t cases[] = {
            {.argc = 1, .argv = {"jpipe", NULL}, .expected=0},
            {.argc = 3, .argv = {"jpipe", "-b", "-1", NULL}, .expected=JP_EBACKLOG_LENGTH},
            {.argc = 3, .argv = {"jpipe", "-b", "", NULL}, .expected=JP_EBACKLOG_LENGTH},
            {.argc = 3, .argv = {"jpipe", "-b", "A", NULL}, .expected=JP_EBACKLOG_LENGTH},
            {.argc = 3, .argv = {"jpipe", "-b", "1025", NULL}, .expected=JP_EBACKLOG_LENGTH},
            {.argc = 3, .argv = {"jpipe", "-b", "0", NULL}, .expected=JP_EBACKLOG_LENGTH},
            {.argc = 3, .argv = {"jpipe", "-b", "1", NULL}, .expected=0,},
            {.argc = 3, .argv = {"jpipe", "--backlog", "1024", NULL}, .expected=0},
            {.argc = 3, .argv = {"jpipe", "--backlog", "-1", NULL}, .expected=JP_EBACKLOG_LENGTH},
            {.argc = 3, .argv = {"jpipe", "--backlog", "", NULL}, .expected=JP_EBACKLOG_LENGTH},
            {.argc = 3, .argv = {"jpipe", "--backlog", "A", NULL}, .expected=JP_EBACKLOG_LENGTH},
            {.argc = 3, .argv = {"jpipe", "--backlog", "1025", NULL}, .expected=JP_EBACKLOG_LENGTH},
            {.argc = 3, .argv = {"jpipe", "--backlog", "0", NULL}, .expected=JP_EBACKLOG_LENGTH},
            {.argc = 3, .argv = {"jpipe", "--backlog", "1", NULL}, .expected=0},
            {.argc = 3, .argv = {"jpipe", "--backlog", "1024", NULL}, .expected=0},
    };

    int len = (sizeof(cases) / sizeof(cases[0]));
    for (int i = 0; i < len; i++) {
        int err = 0;

        optind = 1;
        optarg = NULL;
        opterr = 0;

        err = jp_wrk_exec(cases[i].argc, cases[i].argv);

        JP_ASSERT_EQ(cases[i].expected, err);
    }
}

void test_jp_wrk_exec_args_chunk_size() {
    test_case_t cases[] = {
            {.argc = 1, .argv = {"jpipe", NULL}, .expected=0},
            {.argc = 3, .argv = {"jpipe", "-c", "-1", NULL}, .expected=JP_ECHUNK_SIZE},
            {.argc = 3, .argv = {"jpipe", "-c", "", NULL}, .expected=JP_ECHUNK_SIZE},
            {.argc = 3, .argv = {"jpipe", "-c", "A", NULL}, .expected=JP_ECHUNK_SIZE},
            {.argc = 3, .argv = {"jpipe", "-c", "0kb", NULL}, .expected=JP_ECHUNK_SIZE},
            {.argc = 3, .argv = {"jpipe", "-c", "65537kb", NULL}, .expected=JP_ECHUNK_SIZE},
            {.argc = 3, .argv = {"jpipe", "-c", "10Kb", NULL}, .expected=JP_ECHUNK_SIZE},
            {.argc = 3, .argv = {"jpipe", "-c", "10kbx", NULL}, .expected=JP_ECHUNK_SIZE},
            {.argc = 3, .argv = {"jpipe", "-c", "0mb", NULL}, .expected=JP_ECHUNK_SIZE},
            {.argc = 3, .argv = {"jpipe", "-c", "65mb", NULL}, .expected=JP_ECHUNK_SIZE},
            {.argc = 3, .argv = {"jpipe", "-c", "64Mb", NULL}, .expected=JP_ECHUNK_SIZE},
            {.argc = 3, .argv = {"jpipe", "-c", "64mbx", NULL}, .expected=JP_ECHUNK_SIZE},
            {.argc = 3, .argv = {"jpipe", "-c", "1kb", NULL}, .expected=0},
            {.argc = 3, .argv = {"jpipe", "-c", "10kb", NULL}, .expected=0},
            {.argc = 3, .argv = {"jpipe", "-c", "65536kb", NULL}, .expected=0},
            {.argc = 3, .argv = {"jpipe", "-c", "1mb", NULL}, .expected=0},
            {.argc = 3, .argv = {"jpipe", "-c", "64mb", NULL}, .expected=0},
            {.argc = 3, .argv = {"jpipe", "--chunk-size", "-1", NULL}, .expected=JP_ECHUNK_SIZE},
            {.argc = 3, .argv = {"jpipe", "--chunk-size", "", NULL}, .expected=JP_ECHUNK_SIZE},
            {.argc = 3, .argv = {"jpipe", "--chunk-size", "A", NULL}, .expected=JP_ECHUNK_SIZE},
            {.argc = 3, .argv = {"jpipe", "--chunk-size", "0kb", NULL}, .expected=JP_ECHUNK_SIZE},
            {.argc = 3, .argv = {"jpipe", "--chunk-size", "65537kb", NULL}, .expected=JP_ECHUNK_SIZE},
            {.argc = 3, .argv = {"jpipe", "--chunk-size", "10Kb", NULL}, .expected=JP_ECHUNK_SIZE},
            {.argc = 3, .argv = {"jpipe", "--chunk-size", "10kbx", NULL}, .expected=JP_ECHUNK_SIZE},
            {.argc = 3, .argv = {"jpipe", "--chunk-size", "0mb", NULL}, .expected=JP_ECHUNK_SIZE},
            {.argc = 3, .argv = {"jpipe", "--chunk-size", "65mb", NULL}, .expected=JP_ECHUNK_SIZE},
            {.argc = 3, .argv = {"jpipe", "--chunk-size", "64Mb", NULL}, .expected=JP_ECHUNK_SIZE},
            {.argc = 3, .argv = {"jpipe", "--chunk-size", "64mbx", NULL}, .expected=JP_ECHUNK_SIZE},
            {.argc = 3, .argv = {"jpipe", "--chunk-size", "1kb", NULL}, .expected=0},
            {.argc = 3, .argv = {"jpipe", "--chunk-size", "10kb", NULL}, .expected=0},
            {.argc = 3, .argv = {"jpipe", "--chunk-size", "65536kb", NULL}, .expected=0},
            {.argc = 3, .argv = {"jpipe", "--chunk-size", "1mb", NULL}, .expected=0},
            {.argc = 3, .argv = {"jpipe", "--chunk-size", "64mb", NULL}, .expected=0},
    };

    int len = (sizeof(cases) / sizeof(cases[0]));
    for (int i = 0; i < len; i++) {
        int err = 0;

        optind = 1;
        optarg = NULL;
        opterr = 0;

        err = jp_wrk_exec(cases[i].argc, cases[i].argv);

        JP_ASSERT_EQ(cases[i].expected, err);
    }
}

void test_jp_wrk_exec_args_out_dir() {
    test_case_t cases[] = {
            {.argc = 1, .argv = {"jpipe", NULL}, .expected=0},
            {.argc = 3, .argv = {"jpipe", "-o", "", NULL}, .expected=JP_EOUT_DIR},
            {.argc = 3, .argv = {"jpipe", "-o", "/tmp", NULL}, .expected= 0},
            {.argc = 3, .argv = {"jpipe", "--out-dir", "", NULL}, .expected= JP_EOUT_DIR},
            {.argc = 3, .argv = {"jpipe", "--out-dir", "/tmp", NULL}, .expected= 0},
    };

    int len = (sizeof(cases) / sizeof(cases[0]));
    for (int i = 0; i < len; i++) {
        int err = 0;

        optind = 1;
        optarg = NULL;
        opterr = 0;

        err = jp_wrk_exec(cases[i].argc, cases[i].argv);

        JP_ASSERT_EQ(cases[i].expected, err);
    }
}

void test_jp_wrk_exec_args_inv_cmd() {
    test_case_t cases[] = {
            {.argc = 1, .argv = {"jpipe", NULL}, .expected=0},
            {.argc = 2, .argv = {"jpipe", "-x",  NULL}, .expected=JP_EUNKNOWN_RUN_CMD},
            {.argc = 2, .argv = {"jpipe", "--xx", NULL}, .expected= JP_EUNKNOWN_RUN_CMD},
            {.argc = 4, .argv = {"jpipe", "-b", "100","-x", NULL}, .expected= JP_EUNKNOWN_RUN_CMD},
            {.argc = 4, .argv = {"jpipe", "-b", "100","100", NULL}, .expected= JP_EUNKNOWN_RUN_CMD},
    };

    int len = (sizeof(cases) / sizeof(cases[0]));
    for (int i = 0; i < len; i++) {
        int err = 0;

        optind = 1;
        optarg = NULL;
        opterr = 0;

        err = jp_wrk_exec(cases[i].argc, cases[i].argv);

        JP_ASSERT_EQ(cases[i].expected, err);
    }
}

int main(int argc, char *argv[]) {
    test_jp_wrk_exec_help_short();
    test_jp_wrk_exec_help_long();
    test_jp_wrk_exec_args_chunk_size();
    test_jp_wrk_exec_args_backlog_length();
    test_jp_wrk_exec_args_out_dir();
    test_jp_wrk_exec_args_inv_cmd();
    return EXIT_SUCCESS;
}
