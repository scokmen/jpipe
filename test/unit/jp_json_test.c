#include <jp_errno.h>
#include <jp_json.h>
#include <jp_memory.h>
#include <jp_test.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef struct {
    size_t size;
    const char* input;
    const char* output;
} test_case_t;

void test_jp_json_value_encoder(void) {
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
        const size_t size =
            jp_json_value_encoder((unsigned char*) cases[i].input, strlen(cases[i].input), buffer, 1024);
        JP_ASSERT_EQ(cases[i].size, size);
        JP_ASSERT_OK(memcmp(cases[i].output, buffer, size));
    }
}

void test_jp_json_value_encoder_truncated(void) {
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
        const size_t size = jp_json_value_encoder((unsigned char*) cases[i].input, strlen(cases[i].input), buffer, 16);
        JP_ASSERT_EQ(cases[i].size, size);
        JP_ASSERT_OK(memcmp(cases[i].output, buffer, size));
    }
}

void test_jp_json_prefix_encoder_empty(void) {
    size_t prefix_len                = 0;
    jp_field_set_t* field_set        = jp_field_set_create(1);
    const unsigned char* line_prefix = jp_json_prefix_encoder(field_set, &prefix_len);

    JP_ASSERT_OK(memcmp((char*) line_prefix, "{\"msg\":", prefix_len));
    jp_field_set_destroy(field_set);
    JP_FREE(line_prefix);
}

void test_jp_json_prefix_encoder_ascii_field_values(void) {
    size_t prefix_len         = 0;
    jp_field_set_t* field_set = jp_field_set_create(8);

    JP_ASSERT_OK(jp_field_set_add(field_set, "env=prod"));
    JP_ASSERT_OK(jp_field_set_add(field_set, "host=https://www.example.com/"));
    JP_ASSERT_OK(jp_field_set_add(field_set, "version=1.2.3"));
    const unsigned char* line_prefix = jp_json_prefix_encoder(field_set, &prefix_len);

    JP_ASSERT_OK(memcmp((char*) line_prefix,
                        "{\"env\":\"prod\",\"host\":\"https://www.example.com/\",\"version\":\"1.2.3\",\"msg\":",
                        prefix_len));

    jp_field_set_destroy(field_set);
    JP_FREE(line_prefix);
}

void test_jp_json_prefix_encoder_non_ascii_field_values(void) {
    size_t prefix_len         = 0;
    jp_field_set_t* field_set = jp_field_set_create(8);

    JP_ASSERT_OK(jp_field_set_add(field_set, "label={ŵèéêëěẽēėęřțťþtýŷÿy}"));
    JP_ASSERT_OK(jp_field_set_add(field_set, "headers=h1\th2\th3"));
    JP_ASSERT_OK(jp_field_set_add(field_set, "version=\"v1-🧪\""));
    const unsigned char* line_prefix = jp_json_prefix_encoder(field_set, &prefix_len);

    JP_ASSERT_OK(memcmp(
        (char*) line_prefix,
        "{\"label\":\"{ŵèéêëěẽēėęřțťþtýŷÿy}\",\"headers\":\"h1\\th2\\th3\",\"version\":\"\\\"v1-🧪\\\"\",\"msg\":",
        prefix_len));

    jp_field_set_destroy(field_set);
    JP_FREE(line_prefix);
}

void test_jp_json_prefix_encoder_type_inference_null(void) {
    size_t prefix_len         = 0;
    jp_field_set_t* field_set = jp_field_set_create(8);

    JP_ASSERT_OK(jp_field_set_add(field_set, "k1=null"));
    JP_ASSERT_OK(jp_field_set_add(field_set, "k2='null'"));
    JP_ASSERT_OK(jp_field_set_add(field_set, "k3= null"));
    JP_ASSERT_OK(jp_field_set_add(field_set, "k4=null "));
    const unsigned char* line_prefix = jp_json_prefix_encoder(field_set, &prefix_len);

    JP_ASSERT_OK(memcmp(
        (char*) line_prefix, "{\"k1\":null,\"k2\":\"'null'\",\"k3\":\" null\",\"k4\":\"null \",\"msg\":", prefix_len));

    jp_field_set_destroy(field_set);
    JP_FREE(line_prefix);
}

void test_jp_json_prefix_encoder_type_inference_bool_true(void) {
    size_t prefix_len         = 0;
    jp_field_set_t* field_set = jp_field_set_create(8);

    JP_ASSERT_OK(jp_field_set_add(field_set, "k1=true"));
    JP_ASSERT_OK(jp_field_set_add(field_set, "k2='true'"));
    JP_ASSERT_OK(jp_field_set_add(field_set, "k3=TRUE"));
    JP_ASSERT_OK(jp_field_set_add(field_set, "k4= true"));
    const unsigned char* line_prefix = jp_json_prefix_encoder(field_set, &prefix_len);

    JP_ASSERT_OK(memcmp(
        (char*) line_prefix, "{\"k1\":true,\"k2\":\"'true'\",\"k3\":\"TRUE\",\"k4\":\" true\",\"msg\":", prefix_len));

    jp_field_set_destroy(field_set);
    JP_FREE(line_prefix);
}

void test_jp_json_prefix_encoder_type_inference_bool_false(void) {
    size_t prefix_len         = 0;
    jp_field_set_t* field_set = jp_field_set_create(8);

    JP_ASSERT_OK(jp_field_set_add(field_set, "k1=false"));
    JP_ASSERT_OK(jp_field_set_add(field_set, "k2='false'"));
    JP_ASSERT_OK(jp_field_set_add(field_set, "k3=FALSE"));
    JP_ASSERT_OK(jp_field_set_add(field_set, "k4= false"));
    const unsigned char* line_prefix = jp_json_prefix_encoder(field_set, &prefix_len);

    JP_ASSERT_OK(memcmp((char*) line_prefix,
                        "{\"k1\":false,\"k2\":\"'false'\",\"k3\":\"FALSE\",\"k4\":\" false\",\"msg\":",
                        prefix_len));

    jp_field_set_destroy(field_set);
    JP_FREE(line_prefix);
}

void test_jp_json_prefix_encoder_type_inference_numbers_valid(void) {
    size_t prefix_len         = 0;
    jp_field_set_t* field_set = jp_field_set_create(8);

    JP_ASSERT_OK(jp_field_set_add(field_set, "k1=0"));
    JP_ASSERT_OK(jp_field_set_add(field_set, "k2=0.124"));
    JP_ASSERT_OK(jp_field_set_add(field_set, "k3=-0.34E12"));
    JP_ASSERT_OK(jp_field_set_add(field_set, "k4=12.22E+12"));
    const unsigned char* line_prefix = jp_json_prefix_encoder(field_set, &prefix_len);

    JP_ASSERT_OK(
        memcmp((char*) line_prefix, "{\"k1\":0,\"k2\":0.124,\"k3\":-0.34E12,\"k4\":12.22E+12,\"msg\":", prefix_len));

    jp_field_set_destroy(field_set);
    JP_FREE(line_prefix);
}

void test_jp_json_prefix_encoder_type_inference_numbers_invalid(void) {
    size_t prefix_len         = 0;
    jp_field_set_t* field_set = jp_field_set_create(8);

    JP_ASSERT_OK(jp_field_set_add(field_set, "k1=.4"));
    JP_ASSERT_OK(jp_field_set_add(field_set, "k2=+3"));
    JP_ASSERT_OK(jp_field_set_add(field_set, "k3=123."));
    JP_ASSERT_OK(jp_field_set_add(field_set, "k4=12.22E+12.4"));
    const unsigned char* line_prefix = jp_json_prefix_encoder(field_set, &prefix_len);

    JP_ASSERT_OK(memcmp((char*) line_prefix,
                        "{\"k1\":\".4\",\"k2\":\"+3\",\"k3\":\"123.\",\"k4\":\"12.22E+12.4\",\"msg\":",
                        prefix_len));

    jp_field_set_destroy(field_set);
    JP_FREE(line_prefix);
}

int main(void) {
    test_jp_json_value_encoder();
    test_jp_json_value_encoder_truncated();
    test_jp_json_prefix_encoder_empty();
    test_jp_json_prefix_encoder_ascii_field_values();
    test_jp_json_prefix_encoder_non_ascii_field_values();
    test_jp_json_prefix_encoder_type_inference_null();
    test_jp_json_prefix_encoder_type_inference_bool_true();
    test_jp_json_prefix_encoder_type_inference_bool_false();
    test_jp_json_prefix_encoder_type_inference_numbers_valid();
    test_jp_json_prefix_encoder_type_inference_numbers_invalid();
    return EXIT_SUCCESS;
}
