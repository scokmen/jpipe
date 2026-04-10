#ifndef JPIPE_JP_TEST_H
#define JPIPE_JP_TEST_H

#include <jp_errno.h>

/**
 * @brief Reports a test failure and terminates execution.
 *
 * This macro acts as the core failure handler for the testing suite.
 * It outputs a formatted error message to stderr,
 * including the exact source file and line number where the failure occurred, then exits the process.
 *
 * @param msg The failure message (printf-style format string).
 * @param ... Variadic arguments for the format string.
 *
 * @note This macro calls exit(EXIT_FAILURE), making it suitable for "stop-on-first-fail" testing strategies.
 */
#define JP_TEST_FAIL(msg, ...)                                                               \
    do {                                                                                     \
        fprintf(stderr, "[FAIL]:  %s:%d\n  " msg "\n\n", __FILE__, __LINE__, ##__VA_ARGS__); \
        exit(EXIT_FAILURE);                                                                  \
    } while (0)

/**
 * @brief Asserts that two integer values are equal.
 *
 * Compares 'expected' and 'actual' values. If they differ, it triggers a test failure and prints both values
 * formatted as unsigned long long to ensure compatibility with various integer widths (int, size_t, uint64_t, etc.).
 *
 * @param expected The value you are looking for.
 * @param actual   The value produced by the code under test.
 */
#define JP_ASSERT_EQ(expected, actual)                                                                             \
    do {                                                                                                           \
        if ((expected) != (actual)) {                                                                              \
            JP_TEST_FAIL(                                                                                          \
                "Expected: %llu\nActual  : %llu", (unsigned long long) (expected), (unsigned long long) (actual)); \
        }                                                                                                          \
    } while (0)

/**
 * @brief Asserts that a function or expression returns 0.
 *
 * This is a specialized alias for JP_ASSERT_EQ(0, actual). It is used to verify that system calls,
 * library functions, or internal logic completed successfully without error codes.
 *
 * @param actual The return value to check (typically an int).
 */
#define JP_ASSERT_OK(actual) JP_ASSERT_EQ(0, actual)

/**
 * @brief Asserts that a pointer is not NULL during tests.
 *
 * If the provided pointer 'actual' is NULL, the macro triggers a test failure via JP_TEST_FAIL with a descriptive
 * message. This is essential for verifying successful allocations or valid object retrievals in test suites.
 *
 * @param actual The pointer to check.
 */
#define JP_ASSERT_NONNULL(actual)                             \
    do {                                                      \
        if ((actual) == NULL) {                               \
            JP_TEST_FAIL("Expected NONNULL\n  Actual: NULL"); \
        }                                                     \
    } while (0)

typedef jp_errno_t (*jp_test_fn)(void* ctx);
typedef void* (*jp_test_thread_handler)(void*);

JP_ATTR_NONNULL(1)
JP_ATTR_WRITE_ONLY_N(1, 2)
void jp_test_get_sandbox(char* buffer, size_t size);

JP_ATTR_NONNULL(2, 3)
int jp_test_compare_stdout(jp_test_fn printer, void* ctx, const char* template_file);

#endif  // JPIPE_JP_TEST_H
