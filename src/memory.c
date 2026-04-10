#include <jp_errno.h>
#include <jp_memory.h>
#include <stdlib.h>
#include <string.h>

void* jp_mem_malloc(size_t size) {
    JP_ATTR_ASSUME(size > 0);

    void* p = malloc(size);
    if (JP_ATTR_UNLIKELY(p == NULL)) {
        JP_ERRNO_RAISE_POSIX(JP_ENOMEMORY, errno);
        JP_ERRNO_DUMP();
        abort();
    }
    return p;
}

void* jp_mem_calloc(size_t count, size_t size) {
    JP_ATTR_ASSUME(count > 0);
    JP_ATTR_ASSUME(size > 0);

    void* p = calloc(count, size);
    if (JP_ATTR_UNLIKELY(p == NULL)) {
        JP_ERRNO_RAISE_POSIX(JP_ENOMEMORY, errno);
        JP_ERRNO_DUMP();
        abort();
    }
    return p;
}

void* jp_mem_strdup(const char* src) {
    const size_t len = strlen(src) + 1;
    char* p          = jp_mem_malloc(len);
    memcpy(p, src, len);
    return p;
}