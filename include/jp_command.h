#ifndef JPIPE_JP_COMMAND_H
#define JPIPE_JP_COMMAND_H

#include <jp_errno.h>
#include <stdint.h>

typedef jp_errno_t (*jp_cmd_handler_t)(int argc, char* argv[]);

typedef struct {
    const char* code;
    const char* name;
    jp_cmd_handler_t handler;
} jp_cmd_t;

JP_ATTR_NONNULL(2, 3, 4)
JP_ATTR_READ_ONLY_N(2, 1)
uint8_t jp_cmd_count(int argc, char* argv[], const char* cmd_short, const char* cmd_long);

JP_ATTR_NONNULL(2, 4)
JP_ATTR_READ_ONLY_N(4, 3)
JP_ATTR_READ_ONLY_N(2, 1)
jp_errno_t jp_cmd_exec(int cmdc, jp_cmd_t* cmds, int argc, char* argv[]);

JP_ATTR_NONNULL(2)
JP_ATTR_READ_ONLY_N(2, 1)
jp_errno_t jp_cmd_help(int argc, char* argv[]);

JP_ATTR_NONNULL(2)
JP_ATTR_READ_ONLY_N(2, 1)
jp_errno_t jp_cmd_version(int argc, char* argv[]);

#endif  // JPIPE_JP_COMMAND_H
