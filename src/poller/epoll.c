#include <jp_common.h>
#include <jp_errno.h>
#include <jp_poller.h>

jp_poller_t* jp_poller_create(JP_UNUSED int timeout) {
    return NULL;
}

jp_errno_t jp_poller_poll(JP_UNUSED jp_poller_t* poller, JP_UNUSED int fd) {
    return 0;
}

jp_errno_t jp_poller_wait(JP_UNUSED jp_poller_t* poller) {
    return 0;
}

void jp_poller_destroy(JP_UNUSED jp_poller_t* poller) {
    return;
}