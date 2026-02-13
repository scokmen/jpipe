#include <stdio.h>
#include <string.h>
#include <jp_errno.h>
#include <jp_command.h>
#include <jp_config.h>

int jp_cmd_exec(jp_cmd_t *cmds, int cmdc, int argc, char *argv[]) {
    if (argc < 2) {
        jp_errno_log(JP_EMISSING_CMD);
        return JP_EMISSING_CMD;
    }

    for (int i = 0; i < cmdc; i++) {
        if (JP_CMD_EQ(argv[1], cmds[i].code, cmds[i].name)) {
            return cmds[i].handler(argc - 1, argv + 1);
        }
    }

    jp_errno_log(JP_EUNKNOWN_CMD);
    return JP_EUNKNOWN_CMD;
}

int jp_cmd_help(int argc, char *argv[]) {
    fprintf(stdout, "Usage: jpipe <command> [options]\n\n");
    fprintf(stdout, "A lightweight C pipe-to-JSON logger designed for high-performance stream capture and extensible metadata injection.\n\n");
    fprintf(stdout, "Commands:\n");
    fprintf(stdout, "  run        Process data with configurable chunking and backlog\n");
    fprintf(stdout, "  version    Display version information\n");
    fprintf(stdout, "  help       Show this help message\n\n");
    fprintf(stdout, "Use 'jpipe <command> --help' for more information on a specific command.\n");
    return 0;
}

int jp_cmd_version(int argc, char *argv[]) {
    fprintf(stdout, "v%.16s\n", JP_VERSION);
    return 0;
}

