#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <jp_field.h>
#include <jp_errno.h>
#include <jp_test.h>

typedef struct {
    int expected;
    const char *kv;
    const char *key;
    const char *value;
} test_case_t;

void test_jp_field_set_new(void) {
    size_t cap = 10;
    jp_field_set_t *set = jp_field_set_new(cap);

    JP_ASSERT_EQ(cap, set->cap);
    JP_ASSERT_EQ(0, set->len);
    JP_ASSERT_NONNULL(set->fields);

    jp_field_set_free(set);
}

void test_jp_field_set_add_set_is_full(void) {
    size_t cap = 2;
    jp_field_set_t *set = jp_field_set_new(cap);

    JP_ASSERT_OK(jp_field_set_add(set, "key-1=value-1"));
    JP_ASSERT_OK(jp_field_set_add(set, "key-2=value-2"));
    JP_ASSERT_EQ(JP_ETOO_MANY_FIELD, jp_field_set_add(set, "key-3=value-3"));

    JP_ASSERT_EQ(2, set->len);
    JP_ASSERT_OK(strcmp(set->fields[0]->key, "key-1"));
    JP_ASSERT_OK(strcmp(set->fields[0]->val, "value-1"));
    JP_ASSERT_OK(strcmp(set->fields[1]->key, "key-2"));
    JP_ASSERT_OK(strcmp(set->fields[1]->val, "value-2"));

    jp_field_set_free(set);
}

void test_jp_field_set_add_key_exists(void) {
    size_t cap = 2;
    jp_field_set_t *set = jp_field_set_new(cap);

    JP_ASSERT_OK(jp_field_set_add(set, "key-1=value-1"));
    JP_ASSERT_OK(jp_field_set_add(set, "key-1=value-1"));
    JP_ASSERT_EQ(1, set->len);
    JP_ASSERT_OK(strcmp(set->fields[0]->key, "key-1"));
    JP_ASSERT_OK(strcmp(set->fields[0]->val, "value-1"));

    jp_field_set_free(set);
}

void test_jp_field_set_add_valid_fields(void) {
    size_t cap = 10;
    jp_field_set_t *set = jp_field_set_new(cap);

    test_case_t cases[] = {
            {.kv="key1=value", .key="key1", .value="value", .expected=0},
            {.kv="key2=va", .key="key2", .value="va", .expected=0},
            {.kv="key3= ", .key="key3", .value=" ", .expected=0},
            {.kv="key4= v ", .key="key4", .value=" v ", .expected=0},
            {.kv="key5='val\"", .key="key5", .value="'val\"", .expected=0},
            {.kv="key6=\\uD83E\\uDDEA", .key="key6", .value="\\uD83E\\uDDEA", .expected=0},
            {.kv="key7=ŵèéêëěẽēėęřțťþtýŷÿy", .key="key7", .value="ŵèéêëěẽēėęřțťþtýŷÿy", .expected=0},
            {.kv="01234567890123456789012345678912=v", .key="01234567890123456789012345678912", .value="v", .expected=0},
    };

    int err;
    int len = (sizeof(cases) / sizeof(cases[0]));
    for (int i = 0; i < len; i++) {
        err = jp_field_set_add(set, cases[i].kv);
        JP_ASSERT_EQ(cases[i].expected, err);
        JP_ASSERT_OK(strcmp(set->fields[i]->key, cases[i].key));
        JP_ASSERT_OK(strcmp(set->fields[i]->val, cases[i].value));
    }

    jp_field_set_free(set);
}

void test_jp_field_set_add_invalid_fields(void) {
    size_t cap = 10;
    jp_field_set_t *set = jp_field_set_new(cap);

    test_case_t cases[] = {
            {.kv="", .expected=JP_EINV_FIELD_KEY},
            {.kv="=", .expected=JP_EINV_FIELD_KEY},
            {.kv="=value", .expected=JP_EINV_FIELD_KEY},
            {.kv=" =value", .expected=JP_EINV_FIELD_KEY},
            {.kv=".=value", .expected=JP_EINV_FIELD_KEY},
            {.kv="këy=value", .expected=JP_EINV_FIELD_KEY},
            {.kv="012345678901234567890123456789120=v", .expected=JP_EINV_FIELD_KEY},
    };

    int err;
    int len = (sizeof(cases) / sizeof(cases[0]));
    for (int i = 0; i < len; i++) {
        err = jp_field_set_add(set, cases[i].kv);
        JP_ASSERT_EQ(cases[i].expected, err);
    }

    jp_field_set_free(set);
}

int main() {
    test_jp_field_set_new();
    test_jp_field_set_add_set_is_full();
    test_jp_field_set_add_key_exists();
    test_jp_field_set_add_valid_fields();
    test_jp_field_set_add_invalid_fields();
    return EXIT_SUCCESS;
}
