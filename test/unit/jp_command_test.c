#include <jp_command.h>
#include <jp_test.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct {
    jp_errno_t expected;
    int argc;
    const char* argv[8];
} test_case_t;

typedef struct {
    int argc;
    char** argv;
} test_ctx_t;

jp_errno_t mock_command(JP_ATTR_UNUSED int argc, JP_ATTR_UNUSED char* argv[]) {
    return JP_OK;
}

jp_errno_t help_command_adapter(void* ctx) {
    test_ctx_t* c = ctx;
    return jp_cmd_help(c->argc, c->argv);
}

jp_errno_t version_command_adapter(void* ctx) {
    test_ctx_t* c = ctx;
    return jp_cmd_version(c->argc, c->argv);
}

void test_jp_cmd_help(void) {
    test_ctx_t ctx = {.argc = 0, .argv = NULL};
    int status     = jp_test_compare_stdout(help_command_adapter, &ctx, "help_command_out.tmpl");
    JP_ASSERT_OK(status);
}

void test_jp_cmd_version(void) {
    test_ctx_t ctx = {.argc = 0, .argv = NULL};
    int status     = jp_test_compare_stdout(version_command_adapter, &ctx, "version_command_out.tmpl");
    JP_ASSERT_OK(status);
}

void test_jp_cmd_count(void) {
    test_case_t cases[] = {
        {.argc = 1, .argv = {"jpipe", NULL}, .expected = 0},
        {.argc = 2, .argv = {"jpipe", "-u", NULL}, .expected = 0},
        {.argc = 2, .argv = {"jpipe", "--unknown", NULL}, .expected = 0},
        {.argc = 2, .argv = {"jpipe", "-c", NULL}, .expected = 1},
        {.argc = 3, .argv = {"jpipe", "-c", "--command", NULL}, .expected = 2},
        {.argc = 4, .argv = {"jpipe", "-c", "--command", "-command", NULL}, .expected = 2},
        {.argc = 3, .argv = {"jpipe", "--command", "--c", NULL}, .expected = 1},
        {.argc = 3, .argv = {"jpipe", "command", "--c", NULL}, .expected = 0},
        {.argc = 3, .argv = {"jpipe", "--command", "c", NULL}, .expected = 1},
    };

    int len = sizeof(cases) / sizeof(cases[0]);
    for (int i = 0; i < len; i++) {
        uint8_t count = jp_cmd_count(cases[i].argc, (char**) cases[i].argv, "-c", "--command");
        JP_ASSERT_EQ(cases[i].expected, count);
    }
}

void test_jp_cmd_exec(void) {
    jp_errno_t err;
    jp_cmd_t commands[] = {
        {.code = "-c", .name = "--command", .handler = mock_command},
    };

    test_case_t cases[] = {
        {.argc = 1, .argv = {"jpipe", NULL}, .expected = JP_EMISSING_CMD},
        {.argc = 2, .argv = {"jpipe", "-u", NULL}, .expected = JP_EUNKNOWN_CMD},
        {.argc = 2, .argv = {"jpipe", "--unknown", NULL}, .expected = JP_EUNKNOWN_CMD},
        {.argc = 2, .argv = {"jpipe", "-c", NULL}, .expected = JP_OK},
        {.argc = 2, .argv = {"jpipe", "--command", NULL}, .expected = JP_OK},
    };

    int len = sizeof(cases) / sizeof(cases[0]);
    for (int i = 0; i < len; i++) {
        err = jp_cmd_exec(1, commands, cases[i].argc, (char**) cases[i].argv);
        JP_ASSERT_EQ(cases[i].expected, err);
    }
}

void test_jp_cmd_quiet_flag(void) {
    jp_cmd_t commands[] = {};

    test_case_t cases[] = {
        {.argc = 2, .argv = {"jpipe", "-q", NULL}},
        {.argc = 2, .argv = {"jpipe", "--quiet", NULL}},
    };

    int len = sizeof(cases) / sizeof(cases[0]);
    for (int i = 0; i < len; i++) {
        JP_CONF_SILENT_SET(false);
        jp_cmd_exec(0, commands, cases[i].argc, (char**) cases[i].argv);
        JP_ASSERT_EQ(true, JP_CONF_SILENT_GET());
    }
}

void test_jp_cmd_no_color_flag(void) {
    jp_cmd_t commands[] = {};

    test_case_t cases[] = {
        {.argc = 2, .argv = {"jpipe", "-C", NULL}},
        {.argc = 2, .argv = {"jpipe", "--no-color", NULL}},
    };

    int len = sizeof(cases) / sizeof(cases[0]);
    for (int i = 0; i < len; i++) {
        JP_CONF_NO_COLOR_SET(false);
        jp_cmd_exec(0, commands, cases[i].argc, (char**) cases[i].argv);
        JP_ASSERT_EQ(true, JP_CONF_NO_COLOR_GET());
    }
}

int main(void) {
    test_jp_cmd_exec();
    test_jp_cmd_help();
    test_jp_cmd_version();
    test_jp_cmd_count();
    test_jp_cmd_quiet_flag();
    test_jp_cmd_no_color_flag();
    return EXIT_SUCCESS;
}
