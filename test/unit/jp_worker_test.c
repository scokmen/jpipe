#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <sys/stat.h>
#include <unistd.h>
#include <jp_worker.h>
#include <jp_test.h>
#include <jp_errno.h>

typedef struct {
    jp_errno_t expected;
    int argc;
    const char *argv[128];
} test_case_t;

typedef struct {
    int argc;
    char *argv[10];

} test_ctx_t;

void tear_up_test_dir(const char *base_path) {
    char os_sub_path[JP_PATH_MAX];
    char os_cmd[JP_PATH_MAX + 64];

    snprintf(os_cmd, sizeof(os_cmd), "chmod -R 777 %s 2>/dev/null; rm -rf %s", base_path, base_path);
    JP_ASSERT_OK(system(os_cmd));
    JP_ASSERT_OK(mkdir(base_path, 0755));

    // CASE 1: ENOTDIR
    snprintf(os_sub_path, sizeof(os_sub_path), "%s/c1_file", base_path);
    snprintf(os_cmd, sizeof(os_cmd), "touch %s", os_sub_path);
    JP_ASSERT_OK(system(os_cmd));

    // CASE 2: EACCES
    snprintf(os_sub_path, sizeof(os_sub_path), "%s/c2_no_perm", base_path);
    JP_ASSERT_OK(mkdir(os_sub_path, 0755));
    JP_ASSERT_OK(chmod(os_sub_path, 0000));

    // CASE 3: TARGET ENOTDIR
    snprintf(os_sub_path, sizeof(os_sub_path), "%s/c3_is_file", base_path);
    snprintf(os_cmd, sizeof(os_cmd), "touch %s", os_sub_path);
    JP_ASSERT_OK(system(os_cmd));

    // CASE 4: READONLY
    snprintf(os_sub_path, sizeof(os_sub_path), "%s/c4_read_only", base_path);
    JP_ASSERT_OK(mkdir(os_sub_path, 0555));
}

jp_errno_t command_adapter(void *ctx) {
    test_ctx_t *c = (test_ctx_t *) ctx;
    return jp_wrk_exec(c->argc, c->argv);
}

void test_jp_wrk_exec_help_command_short(void) {
    test_ctx_t ctx = {.argc = 2, .argv = {(char *) "run", (char *) "-h", NULL}};
    int status = jp_test_compare_stdout(command_adapter, &ctx, "run_help_command_out.tmpl");
    JP_ASSERT_OK(status);
}

void test_jp_wrk_exec_help_command_long(void) {
    test_ctx_t ctx = {.argc = 2, .argv = {(char *) "run", (char *) "--help", NULL}};
    int status = jp_test_compare_stdout(command_adapter, &ctx, "run_help_command_out.tmpl");
    JP_ASSERT_OK(status);
}

void test_jp_wrk_exec_buffer_size(void) {
    test_case_t cases[] = {
            {.argc = 1, .argv = {"jpipe", NULL}, .expected=0},
            {.argc = 3, .argv = {"jpipe", "-b", "-1", NULL}, .expected=JP_EBUFFER_SIZE},
            {.argc = 3, .argv = {"jpipe", "-b", "", NULL}, .expected=JP_EBUFFER_SIZE},
            {.argc = 3, .argv = {"jpipe", "-b", "A", NULL}, .expected=JP_EBUFFER_SIZE},
            {.argc = 3, .argv = {"jpipe", "-b", "1025", NULL}, .expected=JP_EBUFFER_SIZE},
            {.argc = 3, .argv = {"jpipe", "-b", "0", NULL}, .expected=JP_EBUFFER_SIZE},
            {.argc = 3, .argv = {"jpipe", "-b", "1", NULL}, .expected=0,},
            {.argc = 3, .argv = {"jpipe", "--buffer-size", "1024", NULL}, .expected=0},
            {.argc = 3, .argv = {"jpipe", "--buffer-size", "-1", NULL}, .expected=JP_EBUFFER_SIZE},
            {.argc = 3, .argv = {"jpipe", "--buffer-size", "", NULL}, .expected=JP_EBUFFER_SIZE},
            {.argc = 3, .argv = {"jpipe", "--buffer-size", "A", NULL}, .expected=JP_EBUFFER_SIZE},
            {.argc = 3, .argv = {"jpipe", "--buffer-size", "1025", NULL}, .expected=JP_EBUFFER_SIZE},
            {.argc = 3, .argv = {"jpipe", "--buffer-size", "0", NULL}, .expected=JP_EBUFFER_SIZE},
            {.argc = 3, .argv = {"jpipe", "--buffer-size", "1", NULL}, .expected=0},
            {.argc = 3, .argv = {"jpipe", "--buffer-size", "1024", NULL}, .expected=0},
    };

    jp_errno_t err;
    int len = (sizeof(cases) / sizeof(cases[0]));
    for (int i = 0; i < len; i++) {
        optind = 1;
        optarg = NULL;
        opterr = 0;

        err = jp_wrk_exec(cases[i].argc, (char **) cases[i].argv);

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

    jp_errno_t err;
    int len = (sizeof(cases) / sizeof(cases[0]));
    for (int i = 0; i < len; i++) {
        optind = 1;
        optarg = NULL;
        opterr = 0;

        err = jp_wrk_exec(cases[i].argc, (char **) cases[i].argv);

        JP_ASSERT_EQ(cases[i].expected, err);
    }
}

void test_jp_wrk_exec_out_dir(void) {
    test_case_t cases[] = {
            {.argc = 1, .argv = {"jpipe", NULL}, .expected=0},
            {.argc = 3, .argv = {"jpipe", "-o", "", NULL}, .expected=JP_EMISSING_CMD},
            {.argc = 3, .argv = {"jpipe", "-o", "/tmp", NULL}, .expected= 0},
            {.argc = 3, .argv = {"jpipe", "--out-dir", "", NULL}, .expected= JP_EMISSING_CMD},
            {.argc = 3, .argv = {"jpipe", "--out-dir", "/tmp", NULL}, .expected= 0},
    };

    jp_errno_t err;
    int len = (sizeof(cases) / sizeof(cases[0]));
    for (int i = 0; i < len; i++) {
        optind = 1;
        optarg = NULL;
        opterr = 0;

        err = jp_wrk_exec(cases[i].argc, (char **) cases[i].argv);

        JP_ASSERT_EQ(cases[i].expected, err);
    }
}

void test_jp_wrk_exec_fields(void) {
    test_case_t cases[] = {
            {
                    .argc = 65,
                    .argv = {"jpipe",
                             "-f", "k=v", "--field", "k=v", "-f", "k=v", "--field", "k=v",
                             "-f", "k=v", "--field", "k=v", "-f", "k=v", "--field", "k=v",
                             "-f", "k=v", "--field", "k=v", "-f", "k=v", "--field", "k=v",
                             "-f", "k=v", "--field", "k=v", "-f", "k=v", "--field", "k=v",
                             "-f", "k=v", "--field", "k=v", "-f", "k=v", "--field", "k=v",
                             "-f", "k=v", "--field", "k=v", "-f", "k=v", "--field", "k=v",
                             "-f", "k=v", "--field", "k=v", "-f", "k=v", "--field", "k=v",
                             "-f", "k=v", "--field", "k=v", "-f", "k=v", "--field", "k=v",
                             NULL},
                    .expected=0
            },
            {
                    .argc = 66,
                    .argv = {"jpipe",
                             "-f", "k=v", "--field", "k=v", "-f", "k=v", "--field", "k=v",
                             "-f", "k=v", "--field", "k=v", "-f", "k=v", "--field", "k=v",
                             "-f", "k=v", "--field", "k=v", "-f", "k=v", "--field", "k=v",
                             "-f", "k=v", "--field", "k=v", "-f", "k=v", "--field", "k=v",
                             "-f", "k=v", "--field", "k=v", "-f", "k=v", "--field", "k=v",
                             "-f", "k=v", "--field", "k=v", "-f", "k=v", "--field", "k=v",
                             "-f", "k=v", "--field", "k=v", "-f", "k=v", "--field", "k=v",
                             "-f", "k=v", "--field", "k=v", "-f", "k=v", "--field", "k=v",
                             "-f", "k=v", NULL},
                    .expected= JP_ETOO_MANY_FIELD
            },
    };

    jp_errno_t err;
    int len = (sizeof(cases) / sizeof(cases[0]));
    for (int i = 0; i < len; i++) {
        optind = 1;
        optarg = NULL;
        opterr = 0;

        err = jp_wrk_exec(cases[i].argc, (char **) cases[i].argv);

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

    jp_errno_t err;
    int len = (sizeof(cases) / sizeof(cases[0]));
    for (int i = 0; i < len; i++) {
        optind = 1;
        optarg = NULL;
        opterr = 0;

        err = jp_wrk_exec(cases[i].argc, (char **) cases[i].argv);

        JP_ASSERT_EQ(cases[i].expected, err);
    }
}

void test_jp_wrk_exec_out_dir_enotdir(void) {
    jp_errno_t err;
    char tmp_dir[JP_PATH_MAX];
    char out_dir[JP_PATH_MAX + 64];

    jp_test_get_sandbox(tmp_dir, sizeof(tmp_dir));
    snprintf(out_dir, sizeof(out_dir), "%s/c1_file/target", tmp_dir);

    const char *args[4] = {"run", "-o", out_dir, NULL};

    tear_up_test_dir(tmp_dir);
    err = jp_wrk_exec(3, (char **) args);

    JP_ASSERT_EQ(JP_EOUT_DIR, err);
}

void test_jp_wrk_exec_out_dir_eacces(void) {
    jp_errno_t err;
    char tmp_dir[JP_PATH_MAX];
    char out_dir[JP_PATH_MAX + 64];

    jp_test_get_sandbox(tmp_dir, sizeof(tmp_dir));
    snprintf(out_dir, sizeof(out_dir), "%s/c2_no_perm/target", tmp_dir);

    const char *args[4] = {"run", "-o", out_dir, NULL};

    tear_up_test_dir(tmp_dir);
    err = jp_wrk_exec(3, (char **) args);

    JP_ASSERT_EQ(getuid() == 0 ? 0 : JP_EOUT_DIR, err);
}

void test_jp_wrk_exec_out_dir_target_enotdir(void) {
    jp_errno_t err;
    char tmp_dir[JP_PATH_MAX];
    char out_dir[JP_PATH_MAX + 64];

    jp_test_get_sandbox(tmp_dir, sizeof(tmp_dir));
    snprintf(out_dir, sizeof(out_dir), "%s/c3_is_file", tmp_dir);

    const char *args[4] = {"run", "-o", out_dir, NULL};

    tear_up_test_dir(tmp_dir);
    err = jp_wrk_exec(3, (char **) args);

    JP_ASSERT_EQ(JP_EOUT_DIR, err);
}

void test_jp_wrk_exec_out_dir_target_readonly(void) {
    jp_errno_t err;
    char tmp_dir[JP_PATH_MAX];
    char out_dir[JP_PATH_MAX + 64];

    jp_test_get_sandbox(tmp_dir, sizeof(tmp_dir));
    snprintf(out_dir, sizeof(out_dir), "%s/c4_read_only/target", tmp_dir);

    const char *args[4] = {"run", "-o", out_dir, NULL};

    tear_up_test_dir(tmp_dir);
    err = jp_wrk_exec(3, (char **) args);

    JP_ASSERT_EQ(getuid() == 0 ? 0 : JP_EOUT_DIR, err);
}

void test_jp_wrk_exec_no_err(void) {
    jp_errno_t err;
    char tmp_dir[JP_PATH_MAX];
    char out_dir[JP_PATH_MAX + 64];

    jp_test_get_sandbox(tmp_dir, sizeof(tmp_dir));
    snprintf(out_dir, sizeof(out_dir), "%s/happy_path", tmp_dir);

    const char *args[10] = {
            "run",
            "-o", out_dir,
            "-b", "100",
            "-c", "2kb",
            NULL
    };

    tear_up_test_dir(tmp_dir);
    err = jp_wrk_exec(7, (char **) args);

    JP_ASSERT_OK(err);
}

int main(void) {
    test_jp_wrk_exec_help_command_short();
    test_jp_wrk_exec_help_command_long();
    test_jp_wrk_exec_chunk_size();
    test_jp_wrk_exec_buffer_size();
    test_jp_wrk_exec_out_dir();
    test_jp_wrk_exec_fields();
    test_jp_wrk_exec_invalid_command();
    test_jp_wrk_exec_out_dir_enotdir();
    test_jp_wrk_exec_out_dir_eacces();
    test_jp_wrk_exec_out_dir_target_enotdir();
    test_jp_wrk_exec_out_dir_target_readonly();
    test_jp_wrk_exec_no_err();
    return EXIT_SUCCESS;
}
