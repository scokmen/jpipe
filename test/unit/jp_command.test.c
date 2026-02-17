#include <stdlib.h>
#include <stdio.h>
#include <jp_command.h>
#include <jp_test.h>

#define MOCK_RETURN_CODE 99

typedef struct {
    int expected;
    int argc;
    char *argv[4];
} test_case_t;

typedef struct {
    int argc;
    char **argv;
} test_ctx_t;

int mock_command() {
    return MOCK_RETURN_CODE;
}

int help_command_adapter(void *ctx) {
    test_ctx_t *c = (test_ctx_t *) ctx;
    return jp_cmd_help(c->argc, c->argv);
}

int version_command_adapter(void *ctx) {
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
            {.argc = 3, .argv = {"jpipe", "-c", NULL}, .expected=MOCK_RETURN_CODE},
            {.argc = 3, .argv = {"jpipe", "--command", NULL}, .expected=MOCK_RETURN_CODE},
    };

    int len = (sizeof(cases) / sizeof(cases[0]));
    for (int i = 0; i < len; i++) {
        int err = jp_cmd_exec(commands, 1, cases[i].argc, cases[i].argv);
        JP_ASSERT_EQ(cases[i].expected, err);
    }
}

int main() {
    test_jp_cmd_exec();
    test_jp_cmd_help();
    test_jp_cmd_version();
    return EXIT_SUCCESS;
}
