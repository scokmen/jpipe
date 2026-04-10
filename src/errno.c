#include <errno.h>
#include <jp_config.h>
#include <jp_errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#ifdef __APPLE__
#include <mach/mach_error.h>
#endif

#define ERROR_MSG_MAX_LEN 128
#define ERROR_ROW_MAX_LEN (ERROR_MSG_MAX_LEN * 5)

typedef struct {
    int line;
    int sys_err;
    const char* file;
    jp_errno_t err;
    jp_errno_src_t src;
    char stack[ERROR_MSG_MAX_LEN];
} global_errno_ctx_t;

static _Thread_local global_errno_ctx_t ctx = {
    .sys_err = 0, .line = 0, .file = "", .err = 0, .src = JP_ERRNO_SRC_NONE, .stack = ""};

static const char* explain_error(const jp_errno_t err) {
    switch (err) {
#define XX(code, msg) \
    case JP_##code: { \
        return msg;   \
    }
        JP_ERRNO_MAP(XX)
#undef XX
        default: {
            return "unknown error";
        }
    }
}

jp_errno_t jp_errno_ctx_set(jp_errno_t err, jp_errno_src_t src, int sys_err, const char* file, int line) {
    ctx.err      = err;
    ctx.src      = src;
    ctx.sys_err  = sys_err;
    ctx.file     = file;
    ctx.line     = line;
    ctx.stack[0] = '\0';
    return err;
}

jp_errno_t jp_errno_ctx_setf(
    jp_errno_t err, jp_errno_src_t src, int sys_err, const char* file, int line, const char* fmt, ...) {
    jp_errno_ctx_set(err, src, sys_err, file, line);

    va_list args;
    va_start(args, fmt);
    vsnprintf(ctx.stack, ERROR_MSG_MAX_LEN, fmt, args);
    va_end(args);

    return err;
}

void jp_errno_ctx_print(void) {
    if (ctx.err == 0) {
        return;
    }
    char stack[ERROR_ROW_MAX_LEN];
    const size_t size = sizeof(stack);
    size_t offset     = 0;

    offset += (size_t) snprintf(
        stack + offset, size - offset, "%s[ERROR]%s: %.128s", JP_TTY_RED, JP_TTY_RST, explain_error(ctx.err));

    if (ctx.stack[0] != '\0') {
        offset += (size_t) snprintf(stack + offset, size - offset, " | %.128s", ctx.stack);
    }
    if (ctx.sys_err > 0) {
        if (ctx.src == JP_ERRNO_SRC_POSIX) {
            offset +=
                (size_t) snprintf(stack + offset, size - offset, " | (E%d) %.128s", ctx.sys_err, strerror(ctx.sys_err));
        }
#ifdef __APPLE__
        if (ctx.src == JP_ERRNO_SRC_MACH) {
            offset += (size_t) snprintf(
                stack + offset, size - offset, " | (E%d) %.128s", ctx.sys_err, mach_error_string(ctx.sys_err));
        }
#endif
    }

    if (ctx.src != JP_ERRNO_SRC_NONE) {
        offset += (size_t) snprintf(stack + offset, size - offset, " | [%.128s@%d]", ctx.file, ctx.line);
    }
    stack[offset++] = '\n';
    stack[offset]   = '\0';
    fputs(stack, stderr);
    fflush(stderr);
}
