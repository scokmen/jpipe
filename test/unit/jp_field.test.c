#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <jp_field.h>
#include <jp_errno.h>
#include <jp_test.h>

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

    JP_ASSERT_OK(jp_field_set_add(set, "key-1", "value-1"));
    JP_ASSERT_OK(jp_field_set_add(set, "key-2", "value-2"));
    JP_ASSERT_EQ(JP_ETOO_MANY_FIELD, jp_field_set_add(set, "key-3", "value-3"));

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

    JP_ASSERT_OK(jp_field_set_add(set, "key", "value"));
    JP_ASSERT_OK(jp_field_set_add(set, "key", "value"));
    JP_ASSERT_EQ(1, set->len);
    JP_ASSERT_OK(strcmp(set->fields[0]->key, "key"));
    JP_ASSERT_OK(strcmp(set->fields[0]->val, "value"));

    jp_field_set_free(set);
}

int main() {
    test_jp_field_set_new();
    test_jp_field_set_add_set_is_full();
    test_jp_field_set_add_key_exists();
    return EXIT_SUCCESS;
}
