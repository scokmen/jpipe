#include <jp_file.h>
#include <unistd.h>

JP_ATTR_MOCKABLE
jp_errno_t jp_file_writev(JP_ATTR_UNUSED jp_file_handler_t* handler, struct iovec* iov, int count) {
    const int fd = STDOUT_FILENO;

    while (count > 0) {
        ssize_t status = writev(fd, iov, count);
        if (status < 0) {
            if (errno == EINTR) {
                continue;
            }
            return JP_ERRNO_RAISE_POSIX(JP_EWRITE_FAILED, errno);
        }

        if (status == 0) {
            return JP_ERRNO_RAISE_POSIX(JP_EWRITE_FAILED, errno);
        }

        while (count > 0 && status >= (ssize_t) iov[0].iov_len) {
            status -= (ssize_t) iov[0].iov_len;
            iov++;
            count--;
        }

        if (count > 0 && status > 0) {
            iov[0].iov_base  = (unsigned char*) iov[0].iov_base + status;
            iov[0].iov_len  -= (size_t) status;
        }
    }

    return 0;
}
