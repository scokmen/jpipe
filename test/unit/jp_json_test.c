#include <jp_errno.h>
#include <jp_json.h>
#include <jp_test.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef struct {
    size_t size;
    const char* input;
    const char* output;
} test_case_t;

void test_jp_json_escape_string(void) {
    const test_case_t cases[] = {
        {.size = 0, .input = "", .output = ""},
        {.size = 1, .input = " ", .output = " "},
        {.size = 10, .input = "json-value", .output = "json-value"},
        {.size = 5, .input = "{str}", .output = "{str}"},
        {.size = 2, .input = "\r", .output = "\\r"},
        {.size = 10, .input = "key\tvalue", .output = "key\\tvalue"},
        {.size = 13, .input = "string\"value", .output = "string\\\"value"},
        {.size = 6, .input = "\x07", .output = "\\u0007"},
        {.size = 10, .input = "\t🧪🧪", .output = "\\t🧪🧪"},
        {.size = 9, .input = "new\nline", .output = "new\\nline"},
    };

    unsigned char buffer[1024] = {0};
    const int len              = sizeof(cases) / sizeof(cases[0]);

    for (int i = 0; i < len; i++) {
        const size_t size = jp_json_escape((unsigned char*) cases[i].input, strlen(cases[i].input), buffer, 1024);
        JP_ASSERT_EQ(cases[i].size, size);
        JP_ASSERT_OK(memcmp(cases[i].output, buffer, size));
    }
}

void test_jp_json_escape_string_truncated(void) {
    const test_case_t cases[] = {
        {.size = 16, .input = "0123456701234567", .output = "0123456701234567"},
        {.size = 16, .input = "01234567012345678", .output = "0123456701234567"},
        {.size = 15, .input = "012345670123456\t", .output = "012345670123456"},
        {.size = 16, .input = "01234567012345\t", .output = "01234567012345\\t"},
        {.size = 11, .input = "01234567012\x07", .output = "01234567012"},
        {.size = 16, .input = "0123456701\x07", .output = "0123456701\\u0007"},
    };

    unsigned char buffer[16] = {0};
    const int len            = sizeof(cases) / sizeof(cases[0]);

    for (int i = 0; i < len; i++) {
        const size_t size = jp_json_escape((unsigned char*) cases[i].input, strlen(cases[i].input), buffer, 16);
        JP_ASSERT_EQ(cases[i].size, size);
        JP_ASSERT_OK(memcmp(cases[i].output, buffer, size));
    }
}

int main(void) {
    test_jp_json_escape_string();
    test_jp_json_escape_string_truncated();
    return EXIT_SUCCESS;
}
