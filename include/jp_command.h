#ifndef JPIPE_JP_COMMAND_H
#define JPIPE_JP_COMMAND_H

#include <string.h>
#include <jp_errno.h>

#define JP_CMD_EQ(cmd, code, name) (!strcmp((cmd), (code)) || !strcmp((cmd), (name)))

typedef jp_errno_t (*jp_cmd_handler_t)(int argc, char *argv[]);

typedef struct {
    const char *code;
    const char *name;
    jp_cmd_handler_t handler;
} jp_cmd_t;

JP_NONNULL_ARG(2, 4)
JP_READ_PTR_SIZE(4, 3)
JP_READ_PTR_SIZE(2, 1)
jp_errno_t jp_cmd_exec(int cmdc, jp_cmd_t *cmds, int argc, char *argv[]);

JP_NONNULL_ARG(2)
JP_READ_PTR_SIZE(2, 1)
jp_errno_t jp_cmd_help(int argc, char *argv[]);

JP_NONNULL_ARG(2)
JP_READ_PTR_SIZE(2, 1)
jp_errno_t jp_cmd_version(int argc, char *argv[]);

#endif //JPIPE_JP_COMMAND_H
