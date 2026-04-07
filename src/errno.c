#include <errno.h>
#include <jp_config.h>
#include <jp_errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#ifdef __APPLE__
#include <mach/mach_error.h>
#endif

static _Thread_local jp_errno_ctx_t ctx = {
    .std_err = 0, .sys_err = 0, .line = 0, .file = "", .err = 0, .src = JP_ERRNO_SRC_NONE, .stack = ""};

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
    ctx.std_err  = errno;
    ctx.sys_err  = sys_err;
    ctx.file     = file;
    ctx.line     = line;
    ctx.stack[0] = '\0';
    errno        = 0;
    return err;
}

jp_errno_t jp_errno_ctx_setf(
    jp_errno_t err, jp_errno_src_t src, int sys_err, const char* file, int line, const char* fmt, ...) {
    jp_errno_ctx_set(err, src, sys_err, file, line);

    va_list args;
    va_start(args, fmt);
    vsnprintf(ctx.stack, JP_ERRNO_STACK_MAX, fmt, args);
    va_end(args);

    return err;
}

void jp_errno_ctx_dump(void) {
    if (ctx.err == 0) {
        return;
    }
    fprintf(stderr,
            "%s[%s]%s: %.128s",
            JP_TTY_RED,
            ctx.src == JP_ERRNO_SRC_NONE ? "ERROR" : "FATAL",
            JP_TTY_RST,
            explain_error(ctx.err));

    if (ctx.stack[0] != '\0') {
        fprintf(stderr, " | %.128s", ctx.stack);
    }
    if (ctx.sys_err > 0) {
        if (ctx.src == JP_ERRNO_SRC_POSIX) {
            fprintf(stderr, " | (E%d) %.128s", ctx.sys_err, strerror(ctx.sys_err));
        }
#ifdef __APPLE__
        if (ctx.src == JP_ERRNO_SRC_MACH) {
            fprintf(stderr, " | (E%d) %.128s", ctx.sys_err, mach_error_string(ctx.sys_err));
        }
#endif
    } else if (ctx.std_err > 0) {
        fprintf(stderr, " | (E%d) %.128s", ctx.std_err, strerror(ctx.std_err));
    }

    if (ctx.src != JP_ERRNO_SRC_NONE) {
        fprintf(stderr, " | [%.128s@%d]", ctx.file, ctx.line);
    }
    fprintf(stderr, "\n");
    fflush(stderr);
}
