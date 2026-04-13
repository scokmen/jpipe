#ifndef JPIPE_JP_FILE_H
#define JPIPE_JP_FILE_H

#include <jp_errno.h>
#include <sys/uio.h>

typedef struct {
    size_t max_byte_per_file;
} jp_file_handler_t;

JP_ATTR_NONNULL(1, 2)
JP_ATTR_READ_ONLY_N(2, 3)
jp_errno_t jp_file_writev(jp_file_handler_t* handler, struct iovec* iov, int count);

#endif  // JPIPE_JP_FILE_H
