#ifndef JPIPE_JP_ERRNO_H
#define JPIPE_JP_ERRNO_H

#include <errno.h>
#include <jp_common.h>

/**
 * @brief Attempts allocation, logs an error on failure, and returns the error code.
 *
 * This is a specialized version of JP_ALLOC. If the allocation (expr) results in NULL,
 * it logs a "Memory Exhausted" error using the logging system and returns the corresponding error code.
 * Ideal for high-level functions where failing an allocation must be reported to the logging system.
 *
 * @param var  Target variable to store the allocation result.
 * @param expr The allocation expression (e.g., malloc, calloc, strdup).
 * @return Returns the result of jp_errno_log_err(JP_ENOMEMORY) on failure.
 */
#define JP_ALLOC_ERRNO(var, expr) JP_ALLOC(var, expr, jp_errno_log_err(JP_ENOMEMORY))

#if (EAGAIN == EWOULDBLOCK)
/**
 * @brief Portable check for "Resource Temporarily Unavailable" errors.
 *
 * In POSIX programming, non-blocking I/O operations may return EAGAIN or
 * EWOULDBLOCK when no data is available. While most modern systems define
 * them as the same value, some architectures treat them as distinct.
 *
 * This macro abstracts that difference, ensuring that the application
 * correctly identifies "try again" scenarios regardless of the platform.
 *
 * @param err The error code to check (usually from 'errno').
 * @return Non-zero (true) if the error is EAGAIN or EWOULDBLOCK, zero otherwise.
 */
#define JP_ERRNO_EAGAIN(err) ((err) == EAGAIN)
#else
/**
 * @brief Portable check for "Resource Temporarily Unavailable" errors.
 *
 * In POSIX programming, non-blocking I/O operations may return EAGAIN or
 * EWOULDBLOCK when no data is available. While most modern systems define
 * them as the same value, some architectures treat them as distinct.
 *
 * This macro abstracts that difference, ensuring that the application
 * correctly identifies "try again" scenarios regardless of the platform.
 *
 * @param err The error code to check (usually from 'errno').
 * @return Non-zero (true) if the error is EAGAIN or EWOULDBLOCK, zero otherwise.
 */
#define JP_ERRNO_EAGAIN(err) ((err) == EAGAIN || (err) == EWOULDBLOCK)
#endif

#define JP_ERRNO_MAP(XX)                                                                                    \
    XX(EMISSING_CMD, "Missing command. Please use 'jpipe --help' to see available commands.")               \
    XX(EUNKNOWN_CMD, "Unknown command. Please use 'jpipe --help' to see available commands.")               \
    XX(EUNKNOWN_RUN_CMD, "Unknown [run] argument. Please use 'jpipe run --help' to see available options.") \
    XX(ECHUNK_SIZE, "Chunk size must be between 1kb (1024B) and 64kb (131072B), default (16kb).")           \
    XX(EBUFFER_SIZE, "Buffer size must be between 1 and 1024, (default: 64)")                               \
    XX(EOVERFLOW_POLICY, "Overflow policy must be 'wait' or 'drop', (default: wait).")                      \
    XX(EOUT_DIR, "Output directory is invalid, inaccessible or path too long.")                             \
    XX(ETOO_MANY_FIELD, "Too many fields. Maximum allowed field number is 32.")                             \
    XX(EINV_FIELD_KEY, "Invalid key. The key must be at most 64 character (allowed: a-z, A-Z, 0-9, _, -).") \
    XX(ERUN_FAILED, "An error occurred. Cannot run the application.")                                       \
    XX(ESHUTTING_DOWN, "The application is shutting down.")                                                 \
    XX(EMSG_SHOULD_DROP, "Message was dropped.")                                                            \
    XX(EREAD_FAILED, "Cannot read the incoming stream.")                                                    \
    XX(ETRYAGAIN, "Try again later.")                                                                       \
    XX(ENOMEMORY, "Could not allocate memory.")

typedef enum {
    JP_OK = 0,
#define XX(code, _) JP_##code,
    JP_ERRNO_MAP(XX)
#undef XX
} jp_errno_t;

jp_errno_t jp_errno_log_err(jp_errno_t err);

JP_ATTR_FORMAT(2)
jp_errno_t jp_errno_log_err_format(jp_errno_t err, const char* fmt, ...);

JP_ATTR_CONST
const char* jp_errno_explain(jp_errno_t err);

#endif  // JPIPE_JP_ERRNO_H
