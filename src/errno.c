#include <errno.h>
#include <jp_config.h>
#include <jp_errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

jp_errno_t jp_errno_log_err(jp_errno_t err) {
    int err_code    = errno;
    errno           = 0;
    const char* msg = jp_errno_explain(err);

    fprintf(
        stderr, "%s%s[jpipe]%s: %sAn error occurred.\n%s", JP_TTY_BLD, JP_TTY_CYN, JP_TTY_RST, JP_TTY_RED, JP_TTY_RST);
    fprintf(stderr, "  %s└─ Caused By: %s%.256s\n", JP_TTY_YLW, JP_TTY_RST, msg);
    if (err_code > 0) {
        fprintf(stderr,
                "  %s└─ Caused By: %sSystem Error (E%d): %.256s\n",
                JP_TTY_YLW,
                JP_TTY_RST,
                err_code,
                strerror(err_code));
    }
    fflush(stderr);
    return err;
}

jp_errno_t jp_errno_log_err_format(jp_errno_t err, const char* fmt, ...) {
    va_list args;
    int err_code    = errno;
    errno           = 0;
    const char* msg = jp_errno_explain(err);

    fprintf(
        stderr, "%s%s[jpipe]%s: %sAn error occurred.\n%s", JP_TTY_BLD, JP_TTY_CYN, JP_TTY_RST, JP_TTY_RED, JP_TTY_RST);
    fprintf(stderr, "  %s└─ Caused By: %s%.256s\n", JP_TTY_YLW, JP_TTY_RST, msg);
    fprintf(stderr, "  %s└─ Caused By: %s", JP_TTY_YLW, JP_TTY_RST);

    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    fprintf(stderr, "\n");

    if (err_code > 0) {
        fprintf(stderr,
                "  %s└─ Caused By: %sSystem Error (E%d): %.256s\n",
                JP_TTY_YLW,
                JP_TTY_RST,
                err_code,
                strerror(err_code));
    }
    fflush(stderr);
    return err;
}

const char* jp_errno_explain(jp_errno_t err) {
    if (err == JP_OK) {
        return "success";
    }
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
