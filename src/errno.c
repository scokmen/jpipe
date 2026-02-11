#include <j_errno.h>

const char *j_errno_explain(j_errno_t err)
{
    if (err == J_OK) {
        return "success";
    }
    switch (err) {
        #define XX(code, msg) case J_##code: { return msg; }
            J_ERRNO_MAP(XX)
        #undef XX
        default: {
            return "unknown error";
        }
    }
}