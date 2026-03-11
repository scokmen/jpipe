#ifndef JPIPE_JP_POLLER_H
#define JPIPE_JP_POLLER_H

#include <jp_common.h>
#include <jp_errno.h>

typedef struct jp_poller jp_poller_t;

JP_MALLOC
JP_USE_RESULT
jp_poller_t* jp_poller_create(int timeout);

JP_NONNULL_ARG(1)
JP_FILE_DESC_READ(2)
jp_errno_t jp_poller_poll(jp_poller_t* poller, int fd);

JP_NONNULL_ARG(1)
jp_errno_t jp_poller_wait(jp_poller_t* poller);

void jp_poller_destroy(jp_poller_t* poller);

#endif  // JPIPE_JP_POLLER_H
