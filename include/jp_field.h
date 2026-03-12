#ifndef JPIPE_JP_FIELD_H
#define JPIPE_JP_FIELD_H

#include <jp_common.h>
#include <jp_errno.h>

#define JP_FIELD_MAX_KEY_LEN 32

typedef struct {
    size_t key_len;
    const char* key;
    const char* val;
} jp_field_t;

typedef struct {
    size_t len;
    size_t cap;
    jp_field_t** fields;
} jp_field_set_t;

JP_ATTR_ALLOCATED
JP_ATTR_USE_RETURN
jp_field_set_t* jp_field_set_create(size_t cap);

JP_ATTR_NONNULL(1, 2)
JP_ATTR_READONLY(2)
jp_errno_t jp_field_set_add(jp_field_set_t* set, const char* kv);

void jp_field_set_destroy(jp_field_set_t* set);

#endif  // JPIPE_JP_FIELD_H
