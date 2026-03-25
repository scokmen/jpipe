#ifndef JPIPE_JP_FIELD_H
#define JPIPE_JP_FIELD_H

#include <jp_common.h>
#include <jp_errno.h>

typedef struct {
    size_t key_len;
    size_t val_len;
    const char* key;
    unsigned const char* val JP_ATTR_BUFFER;
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
JP_ATTR_READ_ONLY(2)
jp_errno_t jp_field_set_add(jp_field_set_t* set, const char* kv);

void jp_field_set_destroy(jp_field_set_t* set);

#endif  // JPIPE_JP_FIELD_H
