#ifndef JPIPE_JP_TEST_H
#define JPIPE_JP_TEST_H

#define PATH_MAX 1024
#define CMD_MAX  4096

#define JP_TEST_FAIL(msg, ...)                                                           \
do {                                                                                     \
    fprintf(stderr, "[FAIL] %s:%d\n  " msg "\n\n", __FILE__, __LINE__, ##__VA_ARGS__);   \
    exit(EXIT_FAILURE);                                                                  \
} while (0)

#define JP_ASSERT_EQ(expected, actual)                                      \
do {                                                                        \
    if ((expected) != (actual)) {                                           \
        JP_TEST_FAIL("Expected: %d\n  Actual  : %d", (expected), (actual)); \
    }                                                                       \
 } while (0)

typedef int (*jp_test_fn)(void *ctx);

int jp_test_compare_stdout(jp_test_fn printer, void *ctx, const char *template_file);

#endif //JPIPE_JP_TEST_H
