#include <jp_errno.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

void jp_errno_log(jp_errno_t err) {
    if (!err) {
        return;
    }
    int err_code = errno;
    const char *errno_msg = jp_errno_explain(err);
    if (err_code == 0) {
        fprintf(stderr, "[jpipe]: %.256s\n", errno_msg);
        return;
    }
    fprintf(stderr, "[jpipe]: %.256s. Error: %.256s\n", errno_msg, strerror(err_code));
    errno = 0;
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