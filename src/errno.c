#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdarg.h>
#include <jp_errno.h>

jp_errno_t jp_errno_log_err(jp_errno_t err) {
    int err_code;
    const char *msg;

    err_code = errno;
    errno = 0;
    msg = jp_errno_explain(err);

    fprintf(stderr, "[jpipe]: An error occurred.\n");
    fprintf(stderr, "\t└─ Caused By: %.256s\n", msg);
    if (err_code > 0) {
        fprintf(stderr, "\t\t└─ Caused By: System Error (E%d): %.256s\n", err_code, strerror(err_code));
    }
    fflush(stderr);
    return err;
}

jp_errno_t jp_errno_log_err_format(jp_errno_t err, const char *fmt, ...) {
    int err_code;
    const char *msg;
    va_list args;

    err_code = errno;
    errno = 0;
    msg = jp_errno_explain(err);

    fprintf(stderr, "[jpipe]: An error occurred.\n");
    fprintf(stderr, "\t└─ Caused By: %.256s\n", msg);
    fprintf(stderr, "\t\t└─ Caused By: ");

    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    fprintf(stderr, "\n");

    if (err_code > 0) {
        fprintf(stderr, "\t\t\t└─ Caused By: System Error (E%d): %.256s\n", err_code, strerror(err_code));
    }
    fflush(stderr);
    return err;
}

const char *jp_errno_explain(jp_errno_t err) {
    if (err == JP_OK) {
        return "success";
    }
    switch (err) {
#define XX(code, msg) case JP_##code: { return msg; }
        JP_ERRNO_MAP(XX)
#undef XX
        default: {
            return "unknown error";
        }
    }
}