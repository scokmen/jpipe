#ifndef JPIPE_JP_FIELD_H
#define JPIPE_JP_FIELD_H

#include <stdbool.h>
#include <stddef.h>
#include <jp_common.h>

typedef struct {
    size_t key_len;
    const char *key;
    const char *val;
} jp_field_t;

JP_MALLOC
JP_NONNULL_ARG(1, 2)
jp_field_t *jp_field_new(const char *key, const char *val);

void jp_field_free(jp_field_t *field);

typedef struct {
    size_t len;
    size_t cap;
    jp_field_t **fields;
} jp_field_set_t;

JP_MALLOC
jp_field_set_t *jp_field_set_new(size_t cap);

JP_NONNULL_ARG(1, 2, 3)
int jp_field_set_add(jp_field_set_t *set, const char *key, const char *value);

void jp_field_set_free(jp_field_set_t *set);

#endif //JPIPE_JP_FIELD_H
