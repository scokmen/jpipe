#ifndef JPIPE_JP_ERRNO_H
#define JPIPE_JP_ERRNO_H

#include <stdarg.h>
#include <jp_common.h>

#define JP_ERRNO_MAP(XX)                                                                                    \
  XX(EMISSING_CMD     , "Missing command. Please use 'jpipe --help' to see available commands.")            \
  XX(EUNKNOWN_CMD     , "Unknown command. Please use 'jpipe --help' to see available commands.")            \
  XX(EUNKNOWN_RUN_CMD , "Unknown 'run' command. Please use 'jpipe run --help' to see available commands.")  \
  XX(ECHUNK_SIZE      , "Chunk size must be between 1kb (1024B) and 64mb (67108864B).")                     \
  XX(EBACKLOG_LENGTH  , "Backlog length must be between 1 and 1024.")                                       \
  XX(EOUT_DIR         , "Output directory is invalid, inaccessible or too long.")                           \
  XX(ENOMEM           , "Could not allocate memory.")

typedef enum {
    JP_OK = 0,
#define XX(code, _) JP_##code,
    JP_ERRNO_MAP(XX)
#undef XX
} jp_errno_t;

jp_errno_t jp_errno_log_err(jp_errno_t err);

JP_PRINT_FUNC(2)
jp_errno_t jp_errno_log_err_format(jp_errno_t err, const char *fmt, ...);

JP_CONST_FUNC
const char *jp_errno_explain(jp_errno_t err);

#endif //JPIPE_JP_ERRNO_H
