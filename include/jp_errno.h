#ifndef JPIPE_JP_ERRNO_H
#define JPIPE_JP_ERRNO_H

#include <errno.h>
#include <jp_common.h>

/**
 * @brief Portable check for "Resource Temporarily Unavailable" errors.
 *
 * In POSIX programming, non-blocking I/O operations may return EAGAIN or
 * EWOULDBLOCK when no data is available. While most modern systems define
 * them as the same value, some architectures treat them as distinct.
 * This macro abstracts that difference, ensuring that the application
 * correctly identifies "try again" scenarios regardless of the platform.
 *
 * @param err The error code to check (usually from 'errno').
 * @return Non-zero (true) if the error is EAGAIN or EWOULDBLOCK, zero otherwise.
 */
#define JP_ERRNO_EAGAIN(err) ((err) == EAGAIN || (EAGAIN != EWOULDBLOCK && (err) == EWOULDBLOCK))

/**
 * @brief Standard error without additional formatting.
 *
 * @param err The jp_errno_t error code.
 * @return Returns the error code 'err'.
 */
#define JP_ERRNO_RAISE(err) jp_errno_ctx_set((err), JP_ERRNO_SRC_NONE, 0, __FILE_NAME__, __LINE__)

/**
 * @brief Standard error with a custom formatted message.
 *
 * @param err The jp_errno_t error code.
 * @param fmt Format string (printf-style).
 * @param ... Additional arguments for the format string.
 * @return Returns the error code 'err'.
 */
#define JP_ERRNO_RAISEF(err, fmt, ...) \
    jp_errno_ctx_setf((err), JP_ERRNO_SRC_NONE, 0, __FILE_NAME__, __LINE__, (fmt), ##__VA_ARGS__)

/**
 * @brief POSIX system error without formatting.
 *
 * @param err The jp_errno_t error code.
 * @param code POSIX return code.
 * @return Returns the error code 'err'.
 */
#define JP_ERRNO_RAISE_POSIX(err, code) jp_errno_ctx_set((err), JP_ERRNO_SRC_POSIX, code, __FILE_NAME__, __LINE__)

/**
 * @brief POSIX system error with a custom formatted context message.
 *
 * @param err The jp_errno_t error code.
 * @param code POSIX return code.
 * @param fmt Format string (printf-style).
 * @param ... Additional arguments for the format string.
 * @return Returns the error code 'err'.
 */
#define JP_ERRNO_RAISE_POSIXF(err, code, fmt, ...) \
    jp_errno_ctx_setf((err), JP_ERRNO_SRC_POSIX, code, __FILE_NAME__, __LINE__, (fmt), ##__VA_ARGS__)

/**
 * @brief Mach kernel error system error without formatting.
 *
 * @param err The jp_errno_t error code.
 * @param kr The Mach kernel return code.
 * @return Returns the error code 'err'.
 */
#define JP_ERRNO_RAISE_MACH(err, kr) jp_errno_ctx_set((err), JP_ERRNO_SRC_MACH, (int) (kr), __FILE_NAME__, __LINE__)

/**
 * @brief Mach kernel error system error with a custom formatted context message.
 *
 * @param err The jp_errno_t error code.
 * @param kr The Mach kernel return code.
 * @param fmt Format string (printf-style).
 * @param ... Additional arguments for the format string.
 * @return Returns the error code 'err'.
 */
#define JP_ERRNO_RAISE_MACHF(err, kr, fmt, ...) \
    jp_errno_ctx_setf((err), JP_ERRNO_SRC_MACH, (int) (kr), __FILE_NAME__, __LINE__, (fmt), ##__VA_ARGS__)

/**
 * @brief Resets or initializes the global error context to a clean state.
 *
 * @return Returns the error code 0.
 */
#define JP_ERRNO_RESET() jp_errno_ctx_set(0, JP_ERRNO_SRC_NONE, 0, __FILE_NAME__, __LINE__)

/**
 * @brief Prints the global error context.
 */
#define JP_ERRNO_DUMP() jp_errno_ctx_print()

#define JP_ERRNO_MAP(XX)                                                                                          \
    XX(EMISSING_CMD, "Missing command. Please use 'jpipe --help' to see available commands.")                     \
    XX(EUNKNOWN_CMD, "Unknown command. Please use 'jpipe --help' to see available commands.")                     \
    XX(EUNKNOWN_RUN_CMD, "Unknown [run] argument. Please use 'jpipe run --help' to see available options.")       \
    XX(ECHUNK_SIZE, "Chunk size must be between 1kb (1024B) and 128kb (131072B), default (16kb).")                \
    XX(EBUFFER_SIZE, "Buffer size must be between 1 and 1024, (default: 64)")                                     \
    XX(EOVERFLOW_POLICY, "Overflow policy must be 'wait' or 'drop', (default: wait).")                            \
    XX(EOUT_DIR, "Output directory is invalid, inaccessible or path too long.")                                   \
    XX(ETOO_MANY_FIELD, "Too many fields. Maximum allowed field number is 32.")                                   \
    XX(EINV_FIELD_KEY, "Invalid field key. The key must be at most 64 character (allowed: a-z, A-Z, 0-9, _, -).") \
    XX(EINV_FIELD_VAL, "Invalid field value. The key must be at most 512 byte.")                                  \
    XX(ESHUTTING_DOWN, "The application is shutting down.")                                                       \
    XX(EMSG_SHOULD_DROP, "Message was dropped.")                                                                  \
    XX(ETRYAGAIN, "Try again later.")                                                                             \
    XX(ERUN_FAILED, "An error occurred. Cannot run the application.")                                             \
    XX(ENOMEMORY, "Could not allocate memory.")                                                                   \
    XX(EREAD_FAILED, "Cannot read the incoming stream.")                                                          \
    XX(EWRITE_FAILED, "Cannot write the stream.")                                                                 \
    XX(ESYS_ERR, "System error occurred.")

typedef enum {
    JP_OK = 0,
#define XX(code, _) JP_##code,
    JP_ERRNO_MAP(XX)
#undef XX
} jp_errno_t;

typedef enum {
    JP_ERRNO_SRC_NONE = 0,
    JP_ERRNO_SRC_POSIX,
    JP_ERRNO_SRC_MACH,
} jp_errno_src_t;

jp_errno_t jp_errno_ctx_set(jp_errno_t err, jp_errno_src_t src, int sys_err, const char* file, int line);

JP_ATTR_FORMAT(6)
jp_errno_t jp_errno_ctx_setf(
    jp_errno_t err, jp_errno_src_t src, int sys_err, const char* file, int line, const char* fmt, ...);

void jp_errno_ctx_print(void);

#endif  // JPIPE_JP_ERRNO_H
