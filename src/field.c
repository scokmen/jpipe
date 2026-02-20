#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stddef.h>
#include <jp_field.h>
#include <jp_common.h>
#include <jp_errno.h>

static bool is_valid_field_key(const char *start, const char *end) {
    unsigned char c;
    while (start < end) {
        c = *start++;
        if ((c >= 97 && c <= 122) || (c >= 65 && c <= 90) || (c >= 48 && c <= 57) || c == 45 || c == 95) {
            continue;
        }
        return false;
    }
    return true;
}

static jp_field_t *create_field(const char *key, const char *val) {
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

static int crete_field_from_kv(const char *kv, jp_field_t **field) {
    size_t key_len;
    char key[JP_FIELD_MAX_KEY_LEN + 1];
    char *eq = strchr(kv, '=');

    *field = NULL;
    if (eq == NULL || eq == kv) {
        return JP_EINV_FIELD_KEY;
    }

    key_len = (size_t) (eq - kv);
    if (key_len > JP_FIELD_MAX_KEY_LEN) {
        return JP_EINV_FIELD_KEY;
    }

    if (!is_valid_field_key(kv, eq)) {
        return JP_EINV_FIELD_KEY;
    }

    memcpy(key, kv, key_len);
    key[key_len] = '\0';

    JP_ALLOC_OR_RET(*field, create_field(key, eq + 1), JP_ENOMEM);
    return 0;
}

static void destroy_field(jp_field_t *field) {
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

int jp_field_set_add(jp_field_set_t *set, const char *kv) {
    if (set->len == set->cap) {
        return JP_ETOO_MANY_FIELD;
    }
    jp_field_t *field, *new_field;
    int err = crete_field_from_kv(kv, &new_field);
    if (err) {
        return err;
    }
    for (int i = 0; i < set->len; i++) {
        field = set->fields[i];
        if (field->key_len == new_field->key_len && !memcmp(field->key, new_field->key, new_field->key_len)) {
            destroy_field(field);
            set->fields[i] = new_field;
            return 0;
        }
    }
    set->fields[set->len] = new_field;
    set->len++;
    return 0;
}

void jp_field_set_free(jp_field_set_t *set) {
    if (set == NULL) {
        return;
    }
    for (int i = 0; i < set->len; i++) {
        destroy_field(set->fields[i]);
    }
    free(set);
}
