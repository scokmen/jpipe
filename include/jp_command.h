#ifndef JPIPE_JP_COMMAND_H
#define JPIPE_JP_COMMAND_H

#include <jp_errno.h>

#define JP_CMD_EQ(cmd, code, name) \
(!strcmp((cmd), (code)) || !strcmp((cmd), (name)))

typedef int (*jp_cmd_handler_t)(int argc, char *argv[]);

typedef struct {
    const char *code;
    const char *name;
    jp_cmd_handler_t handler;
} jp_cmd_t;

int jp_cmd_exec(jp_cmd_t *cmds, int cmdc, int argc, char *argv[]);

int jp_cmd_help(int argc, char *argv[]);

int jp_cmd_version(int argc, char *argv[]);

#endif //JPIPE_JP_COMMAND_H
