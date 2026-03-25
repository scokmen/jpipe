#ifndef JPIPE_JP_JSON_H
#define JPIPE_JP_JSON_H

#include <jp_common.h>

JP_ATTR_NONNULL(1, 3)
JP_ATTR_READ_ONLY_N(1, 2)
JP_ATTR_WRITE_ONLY_N(3, 4)
size_t jp_json_escape(const unsigned char* restrict src, size_t src_len, unsigned char* restrict dst, size_t dst_len);

#endif  // JPIPE_JP_JSON_H
