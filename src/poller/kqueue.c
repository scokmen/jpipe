#include <errno.h>
#include <fcntl.h>
#include <jp_errno.h>
#include <jp_poller.h>
#include <stdlib.h>
#include <sys/event.h>
#include <unistd.h>

struct jp_poller {
    int fd;
    struct kevent event;
    struct timespec ts;
};

jp_poller_t* jp_poller_create(int timeout) {
    jp_poller_t* poller;
    int fd = kqueue();
    if (fd == -1) {
        return NULL;
    }
    JP_ALLOC(poller, malloc(sizeof(struct jp_poller)), NULL);
    poller->fd    = fd;
    poller->event = (struct kevent) {0};
    poller->ts    = (struct timespec) {.tv_sec = timeout / 1000, .tv_nsec = (long) timeout % 1000 * 1000000};
    return poller;
}

jp_errno_t jp_poller_poll(jp_poller_t* poller, int fd) {
    int err;
    struct kevent event;
    int flags = fcntl(fd, F_GETFL, 0);

    if (flags == -1 || fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        return JP_EREAD_FAILED;
    }

    EV_SET(&event, fd, EVFILT_READ, EV_ADD | EV_ENABLE | EV_CLEAR, 0, 0, NULL);

    err = kevent(poller->fd, &event, 1, NULL, 0, NULL);
    if (err == -1) {
        return JP_EREAD_FAILED;
    }
    return 0;
}

jp_errno_t jp_poller_wait(jp_poller_t* poller) {
    int n = kevent(poller->fd, NULL, 0, &poller->event, 1, &poller->ts);
    if (n == 0) {
        return JP_ETRYAGAIN;
    }

    if (n < 0) {
        return errno == EINTR || JP_ERRNO_EAGAIN(errno) ? JP_ETRYAGAIN : JP_EREAD_FAILED;
    }

    if (poller->event.flags & EV_ERROR) {
        return JP_EREAD_FAILED;
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