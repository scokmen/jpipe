#include <getopt.h>
#include <jp_errno.h>
#include <jp_reader.h>
#include <jp_test.h>
#include <jp_worker.h>
#include <jp_writer.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

typedef struct {
    jp_errno_t expected;
    int argc;
    const char* argv[128];
} test_case_t;

typedef struct {
    int argc;
    char* argv[10];
} test_ctx_t;

jp_errno_t reader_err = 0;
jp_errno_t writer_err = 0;

jp_errno_t jp_reader_consume(jp_reader_ctx_t ctx) {
    jp_queue_finalize(ctx.queue);
    return reader_err;
}

jp_errno_t jp_writer_produce(jp_writer_ctx_t ctx) {
    jp_queue_finalize(ctx.queue);
    return writer_err;
}

void tear_up_test_dir(const char* base_path) {
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

jp_errno_t command_adapter(void* ctx) {
    test_ctx_t* c = ctx;
    return jp_wrk_exec(c->argc, c->argv);
}

void test_jp_wrk_exec_help_command_short(void) {
    test_ctx_t ctx   = {.argc = 2, .argv = {(char*) "run", (char*) "-h", NULL}};
    const int status = jp_test_compare_stdout(command_adapter, &ctx, "run_help_command_out.tmpl");
    JP_ASSERT_OK(status);
}

void test_jp_wrk_exec_help_command_long(void) {
    test_ctx_t ctx   = {.argc = 2, .argv = {(char*) "run", (char*) "--help", NULL}};
    const int status = jp_test_compare_stdout(command_adapter, &ctx, "run_help_command_out.tmpl");
    JP_ASSERT_OK(status);
}

void test_jp_wrk_exec_buffer_size(void) {
    const test_case_t cases[] = {
        {.argc = 2, .argv = {"jpipe", "-n", NULL}, .expected = 0},
        {.argc = 4, .argv = {"jpipe", "-n", "-b", "-1", NULL}, .expected = JP_EBUFFER_SIZE},
        {.argc = 4, .argv = {"jpipe", "-n", "-b", "", NULL}, .expected = JP_EBUFFER_SIZE},
        {.argc = 4, .argv = {"jpipe", "-n", "-b", "A", NULL}, .expected = JP_EBUFFER_SIZE},
        {.argc = 4, .argv = {"jpipe", "-n", "-b", "999999999999999999999999", NULL}, .expected = JP_EBUFFER_SIZE},
        {.argc = 4, .argv = {"jpipe", "-n", "-b", "1025", NULL}, .expected = JP_EBUFFER_SIZE},
        {.argc = 4, .argv = {"jpipe", "-n", "-b", "0", NULL}, .expected = JP_EBUFFER_SIZE},
        {
            .argc     = 4,
            .argv     = {"jpipe", "-n", "-b", "1", NULL},
            .expected = 0,
        },
        {.argc = 4, .argv = {"jpipe", "-n", "--buffer-size", "1024", NULL}, .expected = 0},
        {.argc = 4, .argv = {"jpipe", "-n", "--buffer-size", "-1", NULL}, .expected = JP_EBUFFER_SIZE},
        {.argc = 4, .argv = {"jpipe", "-n", "--buffer-size", "", NULL}, .expected = JP_EBUFFER_SIZE},
        {.argc = 4, .argv = {"jpipe", "-n", "--buffer-size", "A", NULL}, .expected = JP_EBUFFER_SIZE},
        {.argc     = 4,
         .argv     = {"jpipe", "-n", "--buffer-size", "999999999999999999999999", NULL},
         .expected = JP_EBUFFER_SIZE},
        {.argc = 4, .argv = {"jpipe", "-n", "--buffer-size", "1025", NULL}, .expected = JP_EBUFFER_SIZE},
        {.argc = 4, .argv = {"jpipe", "-n", "--buffer-size", "0", NULL}, .expected = JP_EBUFFER_SIZE},
        {.argc = 4, .argv = {"jpipe", "-n", "--buffer-size", "1", NULL}, .expected = 0},
        {.argc = 4, .argv = {"jpipe", "-n", "--buffer-size", "1024", NULL}, .expected = 0},
    };

    const int len = sizeof(cases) / sizeof(cases[0]);
    for (int i = 0; i < len; i++) {
        optind = 1;
        optarg = NULL;
        opterr = 0;

        const jp_errno_t err = jp_wrk_exec(cases[i].argc, (char**) cases[i].argv);

        JP_ASSERT_EQ(cases[i].expected, err);
    }
}

void test_jp_wrk_exec_policy(void) {
    const test_case_t cases[] = {
        {.argc = 2, .argv = {"jpipe", "-n", NULL}, .expected = 0},
        {.argc = 4, .argv = {"jpipe", "-n", "-p", "", NULL}, .expected = JP_EOVERFLOW_POLICY},
        {.argc = 4, .argv = {"jpipe", "-n", "-p", "waitt", NULL}, .expected = JP_EOVERFLOW_POLICY},
        {.argc = 4, .argv = {"jpipe", "-n", "-p", "droop", NULL}, .expected = JP_EOVERFLOW_POLICY},
        {.argc = 4, .argv = {"jpipe", "-n", "-p", "wait", NULL}, .expected = 0},
        {.argc = 4, .argv = {"jpipe", "-n", "-p", "drop", NULL}, .expected = 0},
        {.argc = 4, .argv = {"jpipe", "-n", "--policy", "", NULL}, .expected = JP_EOVERFLOW_POLICY},
        {.argc = 4, .argv = {"jpipe", "-n", "--policy", "waitt", NULL}, .expected = JP_EOVERFLOW_POLICY},
        {.argc = 4, .argv = {"jpipe", "-n", "--policy", "droop", NULL}, .expected = JP_EOVERFLOW_POLICY},
        {.argc = 4, .argv = {"jpipe", "-n", "--policy", "wait", NULL}, .expected = 0},
        {.argc = 4, .argv = {"jpipe", "-n", "--policy", "drop", NULL}, .expected = 0},
    };

    const int len = sizeof(cases) / sizeof(cases[0]);
    for (int i = 0; i < len; i++) {
        optind = 1;
        optarg = NULL;
        opterr = 0;

        const jp_errno_t err = jp_wrk_exec(cases[i].argc, (char**) cases[i].argv);

        JP_ASSERT_EQ(cases[i].expected, err);
    }
}

void test_jp_wrk_exec_chunk_size(void) {
    const test_case_t cases[] = {
        {.argc = 2, .argv = {"jpipe", "-n", NULL}, .expected = 0},
        {.argc = 4, .argv = {"jpipe", "-n", "-c", "-1", NULL}, .expected = JP_ECHUNK_SIZE},
        {.argc = 4, .argv = {"jpipe", "-n", "-c", "", NULL}, .expected = JP_ECHUNK_SIZE},
        {.argc = 4, .argv = {"jpipe", "-n", "-c", "A", NULL}, .expected = JP_ECHUNK_SIZE},
        {.argc = 4, .argv = {"jpipe", "-n", "-c", "0kb", NULL}, .expected = JP_ECHUNK_SIZE},
        {.argc = 4, .argv = {"jpipe", "-n", "-c", "129kb", NULL}, .expected = JP_ECHUNK_SIZE},
        {.argc = 4, .argv = {"jpipe", "-n", "-c", "10Kb", NULL}, .expected = JP_ECHUNK_SIZE},
        {.argc = 4, .argv = {"jpipe", "-n", "-c", "10kbx", NULL}, .expected = JP_ECHUNK_SIZE},
        {.argc = 4, .argv = {"jpipe", "-n", "-c", "0mb", NULL}, .expected = JP_ECHUNK_SIZE},
        {.argc = 4, .argv = {"jpipe", "-n", "-c", "1mb", NULL}, .expected = JP_ECHUNK_SIZE},
        {.argc = 4, .argv = {"jpipe", "-n", "-c", "999999999999999999999999kb", NULL}, .expected = JP_ECHUNK_SIZE},
        {.argc = 4, .argv = {"jpipe", "-n", "-c", "1kb", NULL}, .expected = 0},
        {.argc = 4, .argv = {"jpipe", "-n", "-c", "10kb", NULL}, .expected = 0},
        {.argc = 4, .argv = {"jpipe", "-n", "-c", "128kb", NULL}, .expected = 0},
        {.argc = 4, .argv = {"jpipe", "-n", "--chunk-size", "-1", NULL}, .expected = JP_ECHUNK_SIZE},
        {.argc = 4, .argv = {"jpipe", "-n", "--chunk-size", "", NULL}, .expected = JP_ECHUNK_SIZE},
        {.argc = 4, .argv = {"jpipe", "-n", "--chunk-size", "A", NULL}, .expected = JP_ECHUNK_SIZE},
        {.argc = 4, .argv = {"jpipe", "-n", "--chunk-size", "0kb", NULL}, .expected = JP_ECHUNK_SIZE},
        {.argc = 4, .argv = {"jpipe", "-n", "--chunk-size", "129kb", NULL}, .expected = JP_ECHUNK_SIZE},
        {.argc = 4, .argv = {"jpipe", "-n", "--chunk-size", "10Kb", NULL}, .expected = JP_ECHUNK_SIZE},
        {.argc = 4, .argv = {"jpipe", "-n", "--chunk-size", "10kbx", NULL}, .expected = JP_ECHUNK_SIZE},
        {.argc = 4, .argv = {"jpipe", "-n", "--chunk-size", "0mb", NULL}, .expected = JP_ECHUNK_SIZE},
        {.argc = 4, .argv = {"jpipe", "-n", "--chunk-size", "1mb", NULL}, .expected = JP_ECHUNK_SIZE},
        {.argc     = 4,
         .argv     = {"jpipe", "-n", "--chunk-size", "999999999999999999999999kb", NULL},
         .expected = JP_ECHUNK_SIZE},
        {.argc = 4, .argv = {"jpipe", "-n", "--chunk-size", "1kb", NULL}, .expected = 0},
        {.argc = 4, .argv = {"jpipe", "-n", "--chunk-size", "10kb", NULL}, .expected = 0},
        {.argc = 4, .argv = {"jpipe", "-n", "--chunk-size", "128kb", NULL}, .expected = 0},
    };

    const int len = sizeof(cases) / sizeof(cases[0]);
    for (int i = 0; i < len; i++) {
        optind = 1;
        optarg = NULL;
        opterr = 0;

        const jp_errno_t err = jp_wrk_exec(cases[i].argc, (char**) cases[i].argv);

        JP_ASSERT_EQ(cases[i].expected, err);
    }
}

void test_jp_wrk_exec_output(void) {
    const test_case_t cases[] = {
        {.argc = 2, .argv = {"jpipe", "-n", NULL}, .expected = 0},
        {.argc = 4, .argv = {"jpipe", "-n", "-o", "", NULL}, .expected = JP_EOUT_DIR},
        {.argc = 4, .argv = {"jpipe", "-n", "-o", "/tmp", NULL}, .expected = 0},
        {.argc = 4, .argv = {"jpipe", "-n", "--output", "", NULL}, .expected = JP_EOUT_DIR},
        {.argc = 4, .argv = {"jpipe", "-n", "--output", "/tmp", NULL}, .expected = 0},
    };

    const int len = sizeof(cases) / sizeof(cases[0]);
    for (int i = 0; i < len; i++) {
        optind = 1;
        optarg = NULL;
        opterr = 0;

        const jp_errno_t err = jp_wrk_exec(cases[i].argc, (char**) cases[i].argv);

        JP_ASSERT_EQ(cases[i].expected, err);
    }
}

void test_jp_wrk_exec_fields(void) {
    const test_case_t cases[] = {
        {.argc = 66,
         .argv = {"jpipe",   "-n",    "-f",      "k01=v", "--field", "k02=v", "-f",      "k03=v", "--field", "k04=v",
                  "-f",      "k05=v", "--field", "k06=v", "-f",      "k07=v", "--field", "k08=v", "-f",      "k09=v",
                  "--field", "k10=v", "-f",      "k11=v", "--field", "k12=v", "-f",      "k13=v", "--field", "k14=v",
                  "-f",      "k15=v", "--field", "k16=v", "-f",      "k17=v", "--field", "k18=v", "-f",      "k19=v",
                  "--field", "k20=v", "-f",      "k21=v", "--field", "k22=v", "-f",      "k23=v", "--field", "k24=v",
                  "-f",      "k25=v", "--field", "k26=v", "-f",      "k27=v", "--field", "k28=v", "-f",      "k29=v",
                  "--field", "k30=v", "-f",      "k31=v", "--field", "k32=v", NULL},
         .expected = 0},
        {.argc = 68,
         .argv = {"jpipe",   "-n",    "-f",      "k01=v", "--field", "k02=v", "-f",      "k03=v", "--field", "k04=v",
                  "-f",      "k05=v", "--field", "k06=v", "-f",      "k07=v", "--field", "k08=v", "-f",      "k09=v",
                  "--field", "k10=v", "-f",      "k11=v", "--field", "k12=v", "-f",      "k13=v", "--field", "k14=v",
                  "-f",      "k15=v", "--field", "k16=v", "-f",      "k17=v", "--field", "k18=v", "-f",      "k19=v",
                  "--field", "k20=v", "-f",      "k21=v", "--field", "k22=v", "-f",      "k23=v", "--field", "k24=v",
                  "-f",      "k25=v", "--field", "k26=v", "-f",      "k27=v", "--field", "k28=v", "-f",      "k29=v",
                  "--field", "k30=v", "-f",      "k31=v", "--field", "k32=v", "--field", "k33=v", NULL},
         .expected = JP_ETOO_MANY_FIELD},
    };

    const int len = sizeof(cases) / sizeof(cases[0]);
    for (int i = 0; i < len; i++) {
        optind = 1;
        optarg = NULL;
        opterr = 0;

        const jp_errno_t err = jp_wrk_exec(cases[i].argc, (char**) cases[i].argv);

        JP_ASSERT_EQ(cases[i].expected, err);
    }
}

void test_jp_wrk_exec_invalid_command(void) {
    const test_case_t cases[] = {
        {.argc = 2, .argv = {"jpipe", "-n", NULL}, .expected = 0},
        {.argc = 3, .argv = {"jpipe", "-n", "-x", NULL}, .expected = JP_EUNKNOWN_RUN_CMD},
        {.argc = 3, .argv = {"jpipe", "-n", "--xx", NULL}, .expected = JP_EUNKNOWN_RUN_CMD},
        {.argc = 5, .argv = {"jpipe", "-n", "-b", "100", "-x", NULL}, .expected = JP_EUNKNOWN_RUN_CMD},
        {.argc = 5, .argv = {"jpipe", "-n", "-b", "100", "100", NULL}, .expected = JP_EUNKNOWN_RUN_CMD},
    };

    const int len = sizeof(cases) / sizeof(cases[0]);
    for (int i = 0; i < len; i++) {
        optind = 1;
        optarg = NULL;
        opterr = 0;

        const jp_errno_t err = jp_wrk_exec(cases[i].argc, (char**) cases[i].argv);

        JP_ASSERT_EQ(cases[i].expected, err);
    }
}

void test_jp_wrk_exec_output_enotdir(void) {
    char tmp_dir[JP_PATH_MAX];
    char output[JP_PATH_MAX + 64];

    jp_test_get_sandbox(tmp_dir, sizeof(tmp_dir));
    snprintf(output, sizeof(output), "%s/c1_file/target", tmp_dir);

    const char* args[5] = {"run", "-n", "-o", output, NULL};

    tear_up_test_dir(tmp_dir);
    const jp_errno_t err = jp_wrk_exec(4, (char**) args);

    JP_ASSERT_EQ(JP_EOUT_DIR, err);
}

void test_jp_wrk_exec_output_eacces(void) {
    char tmp_dir[JP_PATH_MAX];
    char output[JP_PATH_MAX + 64];

    jp_test_get_sandbox(tmp_dir, sizeof(tmp_dir));
    snprintf(output, sizeof(output), "%s/c2_no_perm/target", tmp_dir);

    const char* args[5] = {"run", "-n", "-o", output, NULL};

    tear_up_test_dir(tmp_dir);
    const jp_errno_t err = jp_wrk_exec(4, (char**) args);

    JP_ASSERT_EQ(getuid() == 0 ? 0 : JP_EOUT_DIR, err);
}

void test_jp_wrk_exec_output_target_enotdir(void) {
    char tmp_dir[JP_PATH_MAX];
    char output[JP_PATH_MAX + 64];

    jp_test_get_sandbox(tmp_dir, sizeof(tmp_dir));
    snprintf(output, sizeof(output), "%s/c3_is_file", tmp_dir);

    const char* args[5] = {"run", "-n", "-o", output, NULL};

    tear_up_test_dir(tmp_dir);
    const jp_errno_t err = jp_wrk_exec(4, (char**) args);

    JP_ASSERT_EQ(JP_EOUT_DIR, err);
}

void test_jp_wrk_exec_output_target_readonly(void) {
    char tmp_dir[JP_PATH_MAX];
    char output[JP_PATH_MAX + 64];

    jp_test_get_sandbox(tmp_dir, sizeof(tmp_dir));
    snprintf(output, sizeof(output), "%s/c4_read_only/target", tmp_dir);

    const char* args[5] = {"run", "-n", "-o", output, NULL};

    tear_up_test_dir(tmp_dir);
    const jp_errno_t err = jp_wrk_exec(4, (char**) args);

    JP_ASSERT_EQ(getuid() == 0 ? 0 : JP_EOUT_DIR, err);
}

void test_jp_wrk_exec_no_err(void) {
    char tmp_dir[JP_PATH_MAX];
    char output[JP_PATH_MAX + 64];

    jp_test_get_sandbox(tmp_dir, sizeof(tmp_dir));
    snprintf(output, sizeof(output), "%s/happy_path-1", tmp_dir);

    const char* args[9] = {"run", "-n", "-o", output, "-b", "100", "-c", "2kb", NULL};

    tear_up_test_dir(tmp_dir);
    const jp_errno_t err = jp_wrk_exec(8, (char**) args);

    JP_ASSERT_OK(err);
}

void test_jp_wrk_exec_no_reader_failed(void) {
    char tmp_dir[JP_PATH_MAX];
    char output[JP_PATH_MAX + 64];

    jp_test_get_sandbox(tmp_dir, sizeof(tmp_dir));
    snprintf(output, sizeof(output), "%s/happy_path-2", tmp_dir);

    const char* args[8] = {"run", "-o", output, "-b", "100", "-c", "2kb", NULL};

    reader_err = JP_EREAD_FAILED;
    writer_err = 0;
    tear_up_test_dir(tmp_dir);
    const jp_errno_t err = jp_wrk_exec(7, (char**) args);

    JP_ASSERT_EQ(JP_ERUN_FAILED, err);
}

void test_jp_wrk_exec_no_writer_failed(void) {
    char tmp_dir[JP_PATH_MAX];
    char output[JP_PATH_MAX + 64];

    jp_test_get_sandbox(tmp_dir, sizeof(tmp_dir));
    snprintf(output, sizeof(output), "%s/happy_path-3", tmp_dir);

    const char* args[8] = {"run", "-o", output, "-b", "100", "-c", "2kb", NULL};

    reader_err = 0;
    writer_err = JP_ENOMEMORY;
    tear_up_test_dir(tmp_dir);
    const jp_errno_t err = jp_wrk_exec(7, (char**) args);

    JP_ASSERT_EQ(JP_ERUN_FAILED, err);
}

int main(void) {
    test_jp_wrk_exec_help_command_short();
    test_jp_wrk_exec_help_command_long();
    test_jp_wrk_exec_chunk_size();
    test_jp_wrk_exec_buffer_size();
    test_jp_wrk_exec_policy();
    test_jp_wrk_exec_output();
    test_jp_wrk_exec_fields();
    test_jp_wrk_exec_invalid_command();
    test_jp_wrk_exec_output_enotdir();
    test_jp_wrk_exec_output_eacces();
    test_jp_wrk_exec_output_target_enotdir();
    test_jp_wrk_exec_output_target_readonly();
    test_jp_wrk_exec_no_err();
    test_jp_wrk_exec_no_reader_failed();
    return EXIT_SUCCESS;
}
