#include <j_errno.h>

const char *j_errno_explain(j_errno_t errno)
{
    if (errno == J_OK) {
        return "success";
    }
    switch (errno) {
        #define XX(code, msg) case J_##code: { return msg; }
            J_ERRNO_MAP(XX)
        #undef XX
        default: {
            return "unknown error";
        }
    }
}