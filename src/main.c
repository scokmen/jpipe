#include <stdlib.h>
#include <jp_command.h>
#include <jp_worker.h>

static jp_cmd_t commands[] = {
        {.code = "run", .name = "run", .handler = jp_wrk_exec},
        {.code = "help", .name = "help", .handler = jp_cmd_help},
        {.code = "version", .name = "version", .handler = jp_cmd_version},
};

int main(int argc, char *argv[]) {
    jp_errno_t err = jp_cmd_exec(2, commands, argc, argv);
    if (err) {
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
