#ifndef JPIPE_JP_ERRNO_H
#define JPIPE_JP_ERRNO_H

#include <jp_common.h>

#define JP_ERRNO_MAP(XX)                                                                        \
  XX(EMISSING_CMD     , "Missing command. Use 'jpipe --help' to see available commands.")       \
  XX(EUNKNOWN_CMD     , "Use 'jpipe --help' to see available commands.")                        \
  XX(EUNKNOWN_RUN_CMD , "Use 'jpipe run --help' to see available commands.")                    \
  XX(ECHUNK_SIZE      , "[-c, --chunk-size] Must be between 1kb (1024B) and 64mb (67108864B).") \
  XX(EBACKLOG_LENGTH  , "[-b, --backlog] Backlog length must be between 1 and 1024.")           \
  XX(EOUT_DIR         , "[-o, --out-dir] Output directory is invalid or inaccessible.")         \
  XX(ENOMEM           , "Could not allocate resources")

typedef enum {
    JP_OK = 0,
#define XX(code, _) JP_##code,
    JP_ERRNO_MAP(XX)
#undef XX
} jp_errno_t;

void jp_errno_log(jp_errno_t err);

const char *jp_errno_explain(jp_errno_t err) JP_CONST_FUNC;

#endif //JPIPE_JP_ERRNO_H
