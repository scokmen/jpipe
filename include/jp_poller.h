#ifndef JPIPE_JP_POLLER_H
#define JPIPE_JP_POLLER_H

#include <jp_common.h>
#include <jp_errno.h>

typedef struct jp_poller jp_poller_t;

JP_ATTR_ALLOCATED
JP_ATTR_USE_RETURN
JP_ATTR_NONNULL(2)
jp_poller_t* jp_poller_create(int timeout, jp_errno_t* err);

JP_ATTR_NONNULL(1)
jp_errno_t jp_poller_poll(jp_poller_t* poller, int fd);

JP_ATTR_NONNULL(1)
jp_errno_t jp_poller_wait(jp_poller_t* poller);

void jp_poller_destroy(jp_poller_t* poller);

#endif  // JPIPE_JP_POLLER_H
