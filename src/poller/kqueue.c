#include <fcntl.h>
#include <jp_errno.h>
#include <jp_memory.h>
#include <jp_poller.h>
#include <mach/mach.h>
#include <stdlib.h>
#include <sys/event.h>
#include <unistd.h>

struct jp_poller {
    int fd;
    struct kevent event;
    struct timespec ts;
};

jp_poller_t* jp_poller_create(int timeout, jp_errno_t* err) {
    const int fd = kqueue();
    if (fd == -1) {
        *err = JP_ERRNO_RAISE_POSIX(JP_EREAD_FAILED, errno);
        return NULL;
    }

    jp_poller_t* poller = jp_mem_malloc(sizeof(struct jp_poller));
    poller->fd          = fd;
    poller->event       = (struct kevent) {0};
    poller->ts          = (struct timespec) {.tv_sec = timeout / 1000, .tv_nsec = (long) timeout % 1000 * 1000000};
    return poller;
}

jp_errno_t jp_poller_poll(jp_poller_t* poller, int fd) {
    struct kevent event;
    const int flags = fcntl(fd, F_GETFL, 0);

    if (flags == -1 || fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        return JP_ERRNO_RAISE_POSIX(JP_EREAD_FAILED, errno);
    }

    EV_SET(&event, fd, EVFILT_READ, EV_ADD | EV_ENABLE | EV_CLEAR, 0, 0, NULL);

    const int err = kevent(poller->fd, &event, 1, NULL, 0, NULL);
    if (err == -1) {
        return JP_ERRNO_RAISE_POSIX(JP_EREAD_FAILED, errno);
    }

    return 0;
}

jp_errno_t jp_poller_wait(jp_poller_t* poller) {
    const int n = kevent(poller->fd, NULL, 0, &poller->event, 1, &poller->ts);
    if (n == 0) {
        return JP_ETRYAGAIN;
    }

    if (n < 0) {
        return errno == EINTR || JP_ERRNO_EAGAIN(errno) ? JP_ETRYAGAIN : JP_ERRNO_RAISE_POSIX(JP_EREAD_FAILED, errno);
    }

    if (poller->event.flags & EV_ERROR) {
        return JP_ERRNO_RAISE_POSIX(JP_EREAD_FAILED, (int) poller->event.data);
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