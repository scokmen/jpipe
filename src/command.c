#include <jp_command.h>
#include <jp_config.h>
#include <jp_errno.h>
#include <stdio.h>
#include <string.h>

static void init_global_flags(int argc, char* argv[]) {
    if (jp_cmd_count(argc, argv, "-q", "--quiet") > 0) {
        JP_CONF_SILENT_SET(true);
    }
    if (jp_cmd_count(argc, argv, "-C", "--no-color") > 0) {
        JP_CONF_NO_COLOR_SET(true);
    }
}

uint8_t jp_cmd_count(int argc, char* argv[], const char* cmd_short, const char* cmd_long) {
    uint8_t count = 0;
    for (int i = 0; i < argc; i++) {
        if (!strcmp(argv[i], cmd_short) || !strcmp(argv[i], cmd_long)) {
            count++;
        }
    }
    return count;
}

jp_errno_t jp_cmd_exec(int cmdc, jp_cmd_t* cmds, int argc, char* argv[]) {
    init_global_flags(argc, argv);
    if (argc < 2) {
        return JP_ERRNO_RAISE(JP_EMISSING_CMD);
    }

    for (int i = 0; i < cmdc; i++) {
        if (!strcmp(argv[1], cmds[i].code) || !strcmp(argv[1], cmds[i].name)) {
            return cmds[i].handler(argc - 1, argv + 1);
        }
    }

    return JP_ERRNO_RAISEF(JP_EUNKNOWN_CMD, "Invalid or incomplete command: \"%.32s\"", argv[1]);
}

jp_errno_t jp_cmd_help(JP_ATTR_UNUSED int argc, JP_ATTR_UNUSED char* argv[]) {
    JP_LOG_INFO("Usage: jpipe <command> [options]");
    JP_LOG_INFO(
        "\nA lightweight C pipe-to-JSON logger designed for high-performance stream capture and extensible metadata "
        "injection.");
    JP_LOG_INFO("\nCommands:");
    JP_LOG_INFO("  run                Process data with configurable options.");
    JP_LOG_INFO("  version            Display version information.");
    JP_LOG_INFO("  help               Show this help message.");
    JP_LOG_INFO("\nGlobal Options:");
    JP_LOG_INFO("  -q, --quiet        Display less output (suppress non-critical logs).");
    JP_LOG_INFO("  -C, --no-color     Disable colored output.");
    JP_LOG_INFO("\nEnvironment Variables:");
    JP_LOG_INFO("  NO_COLOR           If set (to any value), disables colored output.");
    JP_LOG_INFO("                     See https://no-color.org/ for details.");
    JP_LOG_INFO("\nUse 'jpipe <command> --help' for more information on a specific command.");
    JP_LOG_INFO("\nExamples:");
    JP_LOG_INFO("  # Capture stream with 1MB chunks to a specific directory");
    JP_LOG_INFO("  cat data.log | jpipe run -c 1mb -o /tmp/output");
    JP_LOG_INFO("\n  # High-throughput capture with a larger buffer");
    JP_LOG_INFO("  tail -f /var/log/syslog | jpipe run -b 128 -c 64kb");
    JP_LOG_INFO("\nDocumentation & Issues:");
    JP_LOG_INFO("  https://github.com/scoekmen/jpipe");
    return 0;
}

jp_errno_t jp_cmd_version(JP_ATTR_UNUSED int argc, JP_ATTR_UNUSED char* argv[]) {
    JP_LOG_INFO("v%.16s", JP_CONF_VERSION);
    return 0;
}
