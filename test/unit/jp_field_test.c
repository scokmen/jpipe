#include <jp_errno.h>
#include <jp_field.h>
#include <jp_test.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    jp_errno_t expected;
    const char* kv;
    const char* key;
    const char* value;
} test_case_t;

const char* generate_test_kv(const char* key, size_t val_len) {
    size_t key_len   = strlen(key);
    size_t total_len = key_len + val_len + 2;
    char* kv         = calloc(total_len, sizeof(char));
    memset(kv, 'x', total_len);
    memcpy((void*) kv, key, key_len);
    kv[key_len]               = '=';
    kv[key_len + val_len + 1] = '\0';
    return kv;
}

void test_jp_field_set_create(void) {
    size_t cap          = 10;
    jp_field_set_t* set = jp_field_set_create(cap);

    JP_ASSERT_EQ(cap, set->cap);
    JP_ASSERT_EQ(0, set->len);
    JP_ASSERT_NONNULL(set->fields);

    jp_field_set_destroy(set);
}

void test_jp_field_set_add_set_is_full(void) {
    size_t cap          = 2;
    jp_field_set_t* set = jp_field_set_create(cap);

    JP_ASSERT_OK(jp_field_set_add(set, "key-1=value-1"));
    JP_ASSERT_OK(jp_field_set_add(set, "key-2=value-2"));
    JP_ASSERT_EQ(JP_ETOO_MANY_FIELD, jp_field_set_add(set, "key-3=value-3"));

    JP_ASSERT_EQ(2, set->len);
    JP_ASSERT_OK(memcmp(set->fields[0]->key, "key-1", set->fields[0]->key_len));
    JP_ASSERT_OK(memcmp(set->fields[0]->val, "value-1", set->fields[0]->val_len));
    JP_ASSERT_OK(memcmp(set->fields[1]->key, "key-2", set->fields[1]->key_len));
    JP_ASSERT_OK(memcmp(set->fields[1]->val, "value-2", set->fields[1]->val_len));

    jp_field_set_destroy(set);
}

void test_jp_field_set_add_key_exists(void) {
    size_t cap          = 2;
    jp_field_set_t* set = jp_field_set_create(cap);

    JP_ASSERT_OK(jp_field_set_add(set, "key-1=value-1"));
    JP_ASSERT_OK(jp_field_set_add(set, "key-1=value-1"));
    JP_ASSERT_EQ(1, set->len);
    JP_ASSERT_OK(memcmp(set->fields[0]->key, "key-1", set->fields[0]->key_len));
    JP_ASSERT_OK(memcmp(set->fields[0]->val, "value-1", set->fields[0]->val_len));

    jp_field_set_destroy(set);
}

void test_jp_field_set_add_valid_keys(void) {
    size_t cap          = 10;
    jp_field_set_t* set = jp_field_set_create(cap);

    test_case_t cases[] = {
        {.kv = "key1=value", .key = "key1", .value = "value", .expected = 0},
        {.kv = "key2=va", .key = "key2", .value = "va", .expected = 0},
        {.kv = "key3= ", .key = "key3", .value = " ", .expected = 0},
        {.kv = "key4= v ", .key = "key4", .value = " v ", .expected = 0},
        {.kv = "key5='val\"", .key = "key5", .value = "'val\"", .expected = 0},
        {.kv = "key6=\\uD83E\\uDDEA", .key = "key6", .value = "\\uD83E\\uDDEA", .expected = 0},
        {.kv = "key7=ŵèéêëěẽēėęřțťþtýŷÿy", .key = "key7", .value = "ŵèéêëěẽēėęřțťþtýŷÿy", .expected = 0},
        {.kv       = "01234567890123456789012345678912=v",
         .key      = "01234567890123456789012345678912",
         .value    = "v",
         .expected = 0},
    };

    jp_errno_t err;
    int len = sizeof(cases) / sizeof(cases[0]);
    for (int i = 0; i < len; i++) {
        err = jp_field_set_add(set, cases[i].kv);
        JP_ASSERT_EQ(cases[i].expected, err);
        JP_ASSERT_OK(memcmp(set->fields[i]->key, cases[i].key, set->fields[i]->key_len));
        JP_ASSERT_OK(memcmp(set->fields[i]->val, cases[i].value, set->fields[i]->val_len));
    }

    jp_field_set_destroy(set);
}

void test_jp_field_set_add_invalid_keys(void) {
    size_t cap          = 10;
    jp_field_set_t* set = jp_field_set_create(cap);

    test_case_t cases[] = {
        {.kv = "", .expected = JP_EINV_FIELD_KEY},
        {.kv = "=", .expected = JP_EINV_FIELD_KEY},
        {.kv = "=value", .expected = JP_EINV_FIELD_KEY},
        {.kv = " =value", .expected = JP_EINV_FIELD_KEY},
        {.kv = ".=value", .expected = JP_EINV_FIELD_KEY},
        {.kv = "këy=value", .expected = JP_EINV_FIELD_KEY},
        {.kv = "012345678901234567890123456789120=v", .expected = JP_EINV_FIELD_KEY},
    };

    jp_errno_t err;
    int len = sizeof(cases) / sizeof(cases[0]);
    for (int i = 0; i < len; i++) {
        JP_ASSERT_NONNULL(cases[i].kv);
        err = jp_field_set_add(set, cases[i].kv);
        JP_ASSERT_EQ(cases[i].expected, err);
    }

    jp_field_set_destroy(set);
}

void test_jp_field_set_add_value_lengths(void) {
    size_t cap          = 10;
    jp_field_set_t* set = jp_field_set_create(cap);

    test_case_t cases[] = {
        {.kv = generate_test_kv("key-1", 0), .expected = JP_EINV_FIELD_VAL},
        {.kv = generate_test_kv("key-2", 1), .expected = 0},
        {.kv = generate_test_kv("key-3", JP_CONF_FIELD_MAX_VAL), .expected = 0},
        {.kv = generate_test_kv("key-4", JP_CONF_FIELD_MAX_VAL + 1), .expected = JP_EINV_FIELD_VAL},
    };

    jp_errno_t err;
    int len = sizeof(cases) / sizeof(cases[0]);
    for (int i = 0; i < len; i++) {
        JP_ASSERT_NONNULL(cases[i].kv);
        err = jp_field_set_add(set, cases[i].kv);
        JP_ASSERT_EQ(cases[i].expected, err);
        JP_FREE(cases[i].kv);
    }

    jp_field_set_destroy(set);
}

int main(void) {
    test_jp_field_set_create();
    test_jp_field_set_add_set_is_full();
    test_jp_field_set_add_key_exists();
    test_jp_field_set_add_valid_keys();
    test_jp_field_set_add_invalid_keys();
    test_jp_field_set_add_value_lengths();
    return EXIT_SUCCESS;
}
