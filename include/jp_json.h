#ifndef JPIPE_JP_JSON_H
#define JPIPE_JP_JSON_H

#include <jp_common.h>
#include <jp_field.h>

#define JP_JSON_ESCAPE_MUL 6

JP_ATTR_NONNULL(1, 2)
JP_ATTR_ALLOCATED
JP_ATTR_USE_RETURN
JP_ATTR_RET_NONNULL
unsigned char* jp_json_prefix_encoder(const jp_field_set_t* field_set, size_t* dst_len);

JP_ATTR_HOT
JP_ATTR_LEAF
JP_ATTR_NONNULL(1, 3)
JP_ATTR_READ_ONLY_N(1, 2)
JP_ATTR_WRITE_ONLY_N(3, 4)
size_t jp_json_value_encoder(const unsigned char* restrict src,
                             size_t src_len,
                             unsigned char* restrict dst,
                             size_t dst_len);

JP_ATTR_NONNULL(1, 2)
JP_ATTR_ALLOCATED
JP_ATTR_USE_RETURN
JP_ATTR_RET_NONNULL
unsigned char* jp_json_postfix_encoder(const jp_field_set_t* field_set, size_t* dst_len);

#endif  // JPIPE_JP_JSON_H
