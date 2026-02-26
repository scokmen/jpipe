#ifndef JPIPE_JP_FIELD_H
#define JPIPE_JP_FIELD_H

#include <stddef.h>
#include <stdbool.h>
#include <jp_common.h>
#include <jp_errno.h>

#define JP_FIELD_MAX_KEY_LEN 32

typedef struct {
    size_t key_len;
    const char *key;
    const char *val;
} jp_field_t;

typedef struct {
    size_t len;
    size_t cap;
    jp_field_t **fields;
} jp_field_set_t;

JP_MALLOC
JP_USE_RESULT
jp_field_set_t *jp_field_set_new(size_t cap);

JP_NONNULL_ARG(1, 2)
JP_READ_PTR(2)
jp_errno_t jp_field_set_add(jp_field_set_t *set, const char *kv);

void jp_field_set_free(jp_field_set_t *set);

#endif //JPIPE_JP_FIELD_H
