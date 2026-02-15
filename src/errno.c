#include <jp_errno.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

void jp_errno_log(jp_errno_t err) {
    if (!err) {
        return;
    }
    int err_code = errno;
    if (err_code > 0) {
        JP_LOG_ERR("Error: %.256s (E%d)", strerror(err_code), err_code);
    }
    JP_LOG_ERR("%.256s", jp_errno_explain(err));
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