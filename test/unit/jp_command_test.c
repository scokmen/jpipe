#include <stdlib.h>
#include <stdio.h>
#include <jp_command.h>
#include <jp_test.h>

typedef struct {
    jp_errno_t expected;
    int argc;
    const char *argv[4];
} test_case_t;

typedef struct {
    int argc;
    char **argv;
} test_ctx_t;

jp_errno_t mock_command(JP_UNUSED int argc, JP_UNUSED char *argv[]) {
    return JP_OK;
}

jp_errno_t help_command_adapter(void *ctx) {
    test_ctx_t *c = (test_ctx_t *) ctx;
    return jp_cmd_help(c->argc, c->argv);
}

jp_errno_t version_command_adapter(void *ctx) {
    test_ctx_t *c = (test_ctx_t *) ctx;
    return jp_cmd_version(c->argc, c->argv);
}

void test_jp_cmd_help(void) {
    test_ctx_t ctx = {.argc = 0, .argv = NULL};
    int status = jp_test_compare_stdout(help_command_adapter, &ctx, "help_command_out.tmpl");
    JP_ASSERT_OK(status);
}

void test_jp_cmd_version(void) {
    test_ctx_t ctx = {.argc = 0, .argv = NULL};
    int status = jp_test_compare_stdout(version_command_adapter, &ctx, "version_command_out.tmpl");
    JP_ASSERT_OK(status);
}

void test_jp_cmd_exec(void) {
    jp_cmd_t commands[] = {
            {.code = "-c", .name = "--command", .handler = mock_command},
    };

    test_case_t cases[] = {
            {.argc = 1, .argv = {"jpipe", NULL}, .expected=JP_EMISSING_CMD},
            {.argc = 2, .argv = {"jpipe", "-u", NULL}, .expected=JP_EUNKNOWN_CMD},
            {.argc = 2, .argv = {"jpipe", "--unknown", NULL}, .expected=JP_EUNKNOWN_CMD},
            {.argc = 3, .argv = {"jpipe", "-c", NULL}, .expected=JP_OK},
            {.argc = 3, .argv = {"jpipe", "--command", NULL}, .expected=JP_OK},
    };

    int len = (sizeof(cases) / sizeof(cases[0]));
    for (int i = 0; i < len; i++) {
        jp_errno_t err = jp_cmd_exec(1, commands, cases[i].argc, (char **) cases[i].argv);
        JP_ASSERT_EQ(cases[i].expected, err);
    }
}

int main(void) {
    test_jp_cmd_exec();
    test_jp_cmd_help();
    test_jp_cmd_version();
    return EXIT_SUCCESS;
}
