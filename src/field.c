#include <stdlib.h>
#include <string.h>
#include <jp_field.h>
#include <jp_common.h>
#include <jp_errno.h>

jp_field_t *jp_field_new(const char *key, const char *val) {
    jp_field_t *field;
    size_t key_len = strlen(key);
    size_t val_len = strlen(val);

    JP_ALLOC_OR_RET(field, malloc(sizeof(jp_field_t) + key_len + val_len + 2), NULL);

    field->key = ((const char *) field + sizeof(jp_field_t));
    field->val = field->key + key_len + 1;
    field->key_len = key_len;
    memcpy((void *) field->key, key, key_len + 1);
    memcpy((void *) field->val, val, val_len + 1);
    return field;
}

void jp_field_free(jp_field_t *field) {
    JP_FREE_IF_ALLOC(field);
}

jp_field_set_t *jp_field_set_new(size_t cap) {
    jp_field_set_t *set;

    JP_ALLOC_OR_RET(set, malloc(sizeof(jp_field_set_t) + (cap * sizeof(jp_field_t *))), NULL);

    set->len = 0;
    set->cap = cap;
    set->fields = (jp_field_t **) (set + 1);
    return set;
}

int jp_field_set_add(jp_field_set_t *set, const char *key, const char *value) {
    if (set->len == set->cap) {
        return JP_ETOO_MANY_FIELD;
    }
    if (set->len == 0) {
        JP_ALLOC_OR_RET(set->fields[set->len], jp_field_new(key, value), JP_ENOMEM);
        set->len++;
        return 0;
    }
    jp_field_t *field = NULL;
    size_t key_len = strlen(key);
    for (int i = 0; i < set->len; i++) {
        field = set->fields[i];
        if (field->key_len && key_len && !memcmp(field->key, key, key_len)) {
            jp_field_free(field);
            JP_ALLOC_OR_RET(field, jp_field_new(key, value), JP_ENOMEM);
            set->fields[i] = field;
            return 0;
        }
    }
    JP_ALLOC_OR_RET(set->fields[set->len], jp_field_new(key, value), JP_ENOMEM);
    set->len++;
    return 0;
}

void jp_field_set_free(jp_field_set_t *set) {
    if (set == NULL) {
        return;
    }
    for (int i = 0; i < set->len; i++) {
        jp_field_free(set->fields[i]);
    }
    free(set);
}