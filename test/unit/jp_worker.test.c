#include <stdlib.h>
#include <stdio.h>
#include <jp_worker.h>
#include <jp_test.h>
#include <jp_errno.h>
#include <getopt.h>
#include <sys/stat.h>

typedef struct {
    int argc;
    int expected;
    char *argv[10];
    const char *expected_out_dir;
} test_case_t;

typedef struct {
    int argc;
    char *argv[10];
} test_ctx_t;

void tear_up_test_dir(void) {
    system("rm -rf /tmp/rw");
    mkdir("/tmp/rw", 0755);

    // CASE 1: ENOTDIR
    system("touch /tmp/rw/c1_file");

    // CASE 2: EACCES
    mkdir("/tmp/rw/c2_no_perm", 0755);
    system("chmod 000 /tmp/rw/c2_no_perm");

    // CASE 3: TARGET ENOTDIR
    system("touch /tmp/rw/c3_is_file");

    // CASE 4: READONLY
    mkdir("/tmp/rw/c4_read_only", 0755);
    system("chmod 555 /tmp/rw/c4_read_only");
}

void tear_down_test_dir(void) {
    system("chmod -R 777 /tmp/rw 2>/dev/null");
    system("rm -rf /tmp/rw");
}

int command_adapter(void *ctx) {
    test_ctx_t *c = (test_ctx_t *) ctx;
    return jp_wrk_exec(c->argc, c->argv);
}

void test_jp_wrk_exec_help_command_short(void) {
    test_ctx_t ctx = {.argc = 2, .argv = {"run", "-h", NULL}};
    int status = jp_test_compare_stdout(command_adapter, &ctx, "run_help_command_out.tmpl");
    JP_ASSERT_EQ(0, status);
}

void test_jp_wrk_exec_help_command_long(void) {
    test_ctx_t ctx = {.argc = 2, .argv = {"run", "--help", NULL}};
    int status = jp_test_compare_stdout(command_adapter, &ctx, "run_help_command_out.tmpl");
    JP_ASSERT_EQ(0, status);
}

void test_jp_wrk_exec_backlog_length(void) {
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

void test_jp_wrk_exec_chunk_size(void) {
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

void test_jp_wrk_exec_out_dir(void) {
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

void test_jp_wrk_exec_invalid_command(void) {
    test_case_t cases[] = {
            {.argc = 1, .argv = {"jpipe", NULL}, .expected=0},
            {.argc = 2, .argv = {"jpipe", "-x", NULL}, .expected=JP_EUNKNOWN_RUN_CMD},
            {.argc = 2, .argv = {"jpipe", "--xx", NULL}, .expected= JP_EUNKNOWN_RUN_CMD},
            {.argc = 4, .argv = {"jpipe", "-b", "100", "-x", NULL}, .expected= JP_EUNKNOWN_RUN_CMD},
            {.argc = 4, .argv = {"jpipe", "-b", "100", "100", NULL}, .expected= JP_EUNKNOWN_RUN_CMD},
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

void test_jp_wrk_exec_out_dir_enotdir(void) {
    int err;
    char *args[4] = {"run", "-o", "/tmp/rw/c1_file/target", NULL};

    tear_up_test_dir();
    err = jp_wrk_exec(3, args);
    tear_down_test_dir();

    JP_ASSERT_EQ(err, JP_EOUT_DIR);
}

void test_jp_wrk_exec_out_dir_eacces(void) {
    int err;
    char *args[4] = {"run", "-o", "/tmp/rw/c2_no_perm/target", NULL};

    tear_up_test_dir();
    err = jp_wrk_exec(3, args);
    tear_down_test_dir();

    JP_ASSERT_EQ(err, JP_EOUT_DIR);
}

void test_jp_wrk_exec_out_dir_target_enotdir(void) {
    int err;
    char *args[4] = {"run", "-o", "/tmp/rw/c3_is_file", NULL};

    tear_up_test_dir();
    err = jp_wrk_exec(3, args);
    tear_down_test_dir();

    JP_ASSERT_EQ(err, JP_EOUT_DIR);
}

void test_jp_wrk_exec_out_dir_target_readonly(void) {
    int err;
    char *args[4] = {"run", "-o", "/tmp/rw/c4_read_only/target", NULL};

    tear_up_test_dir();
    err = jp_wrk_exec(3, args);
    tear_down_test_dir();
    
    JP_ASSERT_EQ(err, JP_EOUT_DIR);
}

void test_jp_wrk_exec_no_err(void) {
    int err;
    char *args[10] = {
            "run",
            "-o", "/tmp/rw/target",
            "-b", "100",
            "-c", "2kb",
            NULL
    };

    tear_up_test_dir();
    err = jp_wrk_exec(7, args);
    tear_down_test_dir();
    
    JP_ASSERT_EQ(err, 0);
}

int main(int argc, char *argv[]) {
    test_jp_wrk_exec_help_command_short();
    test_jp_wrk_exec_help_command_long();
    test_jp_wrk_exec_chunk_size();
    test_jp_wrk_exec_backlog_length();
    test_jp_wrk_exec_out_dir();
    test_jp_wrk_exec_invalid_command();
    test_jp_wrk_exec_out_dir_enotdir();
    test_jp_wrk_exec_out_dir_eacces();
    test_jp_wrk_exec_out_dir_target_enotdir();
    test_jp_wrk_exec_out_dir_target_readonly();
    test_jp_wrk_exec_no_err();
    return EXIT_SUCCESS;
}
