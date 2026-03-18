#include <errno.h>
#include <jp_common.h>
#include <jp_errno.h>
#include <jp_poller.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <unistd.h>

struct jp_poller {
    int fd;
    int timeout;
    struct epoll_event events[1];
};

jp_poller_t* jp_poller_create(int timeout) {
    jp_poller_t* poller;
    int fd = epoll_create1(EPOLL_CLOEXEC);
    if (fd == -1) {
        return NULL;
    }

    JP_ALLOC(poller, malloc(sizeof(struct jp_poller)), NULL);
    poller->fd      = fd;
    poller->timeout = timeout;
    return poller;
}

jp_errno_t jp_poller_poll(jp_poller_t* poller, int fd) {
    struct epoll_event ev;
    ev.events  = EPOLLIN | EPOLLET;
    ev.data.fd = fd;

    if (epoll_ctl(poller->fd, EPOLL_CTL_ADD, fd, &ev) == -1) {
        return JP_EREAD_FAILED;
    }
    return 0;
}

jp_errno_t jp_poller_wait(jp_poller_t* poller) {
    int n = epoll_wait(poller->fd, poller->events, 1, poller->timeout);

    if (n == 0) {
        return JP_ETRYAGAIN;
    }

    if (n == -1) {
        return errno == EINTR || JP_ERRNO_EAGAIN(errno) ? JP_ETRYAGAIN : JP_EREAD_FAILED;
    }

    return 0;
}

void jp_poller_destroy(jp_poller_t* poller) {
    if (poller == NULL) {
        return;
    }
    if (poller->fd >= 0) {
        close(poller->fd);
    }
    JP_FREE(poller);
}