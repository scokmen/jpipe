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

    jp_cmd_invalid(argv[1]);
    jp_errno_log(JP_EUNKNOWN_CMD);
    return JP_EUNKNOWN_CMD;
}

int jp_cmd_help(int argc, char *argv[]) {
    JP_LOG_OUT("Usage: jpipe <command> [options]\n");
    JP_LOG_OUT("A lightweight C pipe-to-JSON logger designed for high-performance stream capture and extensible metadata injection.\n");
    JP_LOG_OUT("Commands:");
    JP_LOG_OUT("  run        Process data with configurable chunking and backlog");
    JP_LOG_OUT("  version    Display version information");
    JP_LOG_OUT("  help       Show this help message\n");
    JP_LOG_OUT("Use 'jpipe <command> --help' for more information on a specific command.\n");
    JP_LOG_OUT("Examples:");
    JP_LOG_OUT("  # Capture stream with 1MB chunks to a specific directory");
    JP_LOG_OUT("  cat data.log | jpipe run -c 1mb -o /tmp/output\n");
    JP_LOG_OUT("  # High-throughput capture with a larger backlog");
    JP_LOG_OUT("  tail -f /var/log/syslog | jpipe run -b 128 -c 64kb");
    return 0;
}

int jp_cmd_version(int argc, char *argv[]) {
    JP_LOG_OUT("v%.16s", JP_VERSION);
    return 0;
}

void jp_cmd_invalid(const char *cmd) {
    JP_LOG_ERR("Error: Invalid or incomplete command: '%s'", cmd);
}
