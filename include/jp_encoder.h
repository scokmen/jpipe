#ifndef JPIPE_JP_ENCODER_H
#define JPIPE_JP_ENCODER_H

#include <jp_field.h>

typedef unsigned char* (*jp_encode_prefix)(const jp_field_set_t* field_set, size_t* out_len);

typedef size_t (*jp_encode_value)(const unsigned char* restrict src,
                                  size_t src_len,
                                  unsigned char* restrict dst,
                                  size_t dst_len);

typedef unsigned char* (*jp_encode_postfix)(const jp_field_set_t* field_set, size_t* out_len);

typedef struct {
    jp_encode_prefix prefix_encoder;
    jp_encode_value value_encoder;
    jp_encode_postfix postfix_encoder;
} jp_encoder_t;

extern const jp_encoder_t jp_encoder_json;

#endif  // JPIPE_JP_ENCODER_H
