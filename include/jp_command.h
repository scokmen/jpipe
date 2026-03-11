#ifndef JPIPE_JP_COMMAND_H
#define JPIPE_JP_COMMAND_H

#include <jp_errno.h>

/**
 * @brief Checks if a command string matches either a short code or a long name.
 *
 * Commonly used for parsing command-line arguments or internal command dispatchers.
 * It performs a logical OR between two string comparisons.
 *
 * @param cmd  The input string to check (e.g., argv[1]).
 * @param code The short version of the command (e.g., "-v").
 * @param name The long version of the command (e.g., "--version").
 * @return Non-zero (true) if 'cmd' matches either 'code' or 'name', zero otherwise.
 * @note This macro uses strcmp, so it is case-sensitive. Ensure 'cmd' is not NULL
 * before calling to avoid segmentation faults.
 */
#define JP_CMD_EQ(cmd, code, name) (!strcmp((cmd), (code)) || !strcmp((cmd), (name)))

typedef jp_errno_t (*jp_cmd_handler_t)(int argc, char* argv[]);

typedef struct {
    const char* code;
    const char* name;
    jp_cmd_handler_t handler;
} jp_cmd_t;

JP_NONNULL_ARG(2, 4)
JP_READ_PTR_SIZE(4, 3)
JP_READ_PTR_SIZE(2, 1)
jp_errno_t jp_cmd_exec(int cmdc, jp_cmd_t* cmds, int argc, char* argv[]);

JP_NONNULL_ARG(2)
JP_READ_PTR_SIZE(2, 1)
jp_errno_t jp_cmd_help(int argc, char* argv[]);

JP_NONNULL_ARG(2)
JP_READ_PTR_SIZE(2, 1)
jp_errno_t jp_cmd_version(int argc, char* argv[]);

#endif  // JPIPE_JP_COMMAND_H
