#ifndef JPIPE_MEMORY_H
#define JPIPE_MEMORY_H

#include <jp_common.h>
#include <stdio.h>

/**
 * @brief Safely frees a pointer and sets it to NULL.
 *
 * Checks if the pointer is not NULL before calling free(). After freeing,
 * it explicitly sets the pointer to NULL to prevent dangling pointer issues and accidental double-free errors.
 *
 * @note The cast to (void*) ensures compatibility with various pointer types
 * and avoids potential warnings with 'const' pointers.
 *
 * @param ptr The pointer to be freed (must be a lvalue).
 */
#define JP_FREE(ptr)             \
    do {                         \
        if ((ptr) != NULL) {     \
            free((void*) (ptr)); \
            (ptr) = NULL;        \
        }                        \
    } while (0)

JP_ATTR_RET_NONNULL
JP_ATTR_USE_RETURN
JP_ATTR_ALLOCS(1)
void* jp_mem_malloc(size_t size);

JP_ATTR_RET_NONNULL
JP_ATTR_USE_RETURN
JP_ATTR_ALLOCS(1, 2)
void* jp_mem_calloc(size_t count, size_t size);

JP_ATTR_RET_NONNULL
JP_ATTR_USE_RETURN
JP_ATTR_NONNULL(1)
JP_ATTR_READ_ONLY(1)
void* jp_mem_strdup(const char* src);

#endif  // JPIPE_MEMORY_H
