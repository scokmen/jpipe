#include <jp_encoder.h>
#include <jp_json.h>
#include <jp_memory.h>
#include <stdlib.h>
#include <string.h>

const jp_encoder_t jp_encoder_json = {.prefix_encoder  = jp_json_prefix_encoder,
                                      .value_encoder   = jp_json_value_encoder,
                                      .postfix_encoder = jp_json_postfix_encoder,
                                      .escaping_mul    = JP_JSON_ESCAPE_MUL};

typedef enum {
    JSON_NUM_STATE_START,
    JSON_NUM_STATE_MINUS,
    JSON_NUM_STATE_ZERO,
    JSON_NUM_STATE_DIGIT,
    JSON_NUM_STATE_DOT,
    JSON_NUM_STATE_FRACTION,
    JSON_NUM_STATE_EXP,
    JSON_NUM_STATE_EXP_SIGN,
    JSON_NUM_STATE_EXP_DIGIT
} json_number_state_t;

static const unsigned char json_size_lut[256] = {
    6, 6, 6, 6, 6, 6, 6, 6, 2, 2, 2, 6, 2, 2, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 1, 1, 2, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 6, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};

static const unsigned char json_short_lookup[256] = {
    ['\n'] = 'n', ['\r'] = 'r', ['\t'] = 't', ['\b'] = 'b', ['\f'] = 'f', ['"'] = '"', ['\\'] = '\\'};

static const unsigned char hex_chars[16] = {
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};

static bool is_json_numeric_literal(const unsigned char* str, size_t len) {
    unsigned char ch;
    json_number_state_t state = JSON_NUM_STATE_START;
    for (size_t i = 0; i < len; i++) {
        ch = str[i];
        switch (state) {
            case JSON_NUM_STATE_START: {
                if (ch == '-') {
                    state = JSON_NUM_STATE_MINUS;
                } else if (ch == '0') {
                    state = JSON_NUM_STATE_ZERO;
                } else if (ch >= '0' && ch <= '9') {
                    state = JSON_NUM_STATE_DIGIT;
                } else {
                    return false;
                }
                break;
            }
            case JSON_NUM_STATE_MINUS: {
                if (ch == '0') {
                    state = JSON_NUM_STATE_ZERO;
                } else if (ch >= '0' && ch <= '9') {
                    state = JSON_NUM_STATE_DIGIT;
                } else {
                    return false;
                }
                break;
            }
            case JSON_NUM_STATE_ZERO: {
                if (ch == '.') {
                    state = JSON_NUM_STATE_DOT;
                } else if (ch == 'e' || ch == 'E') {
                    state = JSON_NUM_STATE_EXP;
                } else {
                    return false;
                }
                break;
            }
            case JSON_NUM_STATE_DIGIT: {
                if (ch >= '0' && ch <= '9') {
                    state = JSON_NUM_STATE_DIGIT;
                } else if (ch == '.') {
                    state = JSON_NUM_STATE_DOT;
                } else if (ch == 'e' || ch == 'E') {
                    state = JSON_NUM_STATE_EXP;
                } else {
                    return false;
                }
                break;
            }
            case JSON_NUM_STATE_DOT: {
                if (ch >= '0' && ch <= '9') {
                    state = JSON_NUM_STATE_FRACTION;
                } else {
                    return false;
                }
                break;
            }
            case JSON_NUM_STATE_FRACTION: {
                if (ch >= '0' && ch <= '9') {
                    state = JSON_NUM_STATE_FRACTION;
                } else if (ch == 'e' || ch == 'E') {
                    state = JSON_NUM_STATE_EXP;
                } else {
                    return false;
                }
                break;
            }
            case JSON_NUM_STATE_EXP: {
                if (ch == '+' || ch == '-') {
                    state = JSON_NUM_STATE_EXP_SIGN;
                } else if (ch >= '0' && ch <= '9') {
                    state = JSON_NUM_STATE_EXP_DIGIT;
                } else {
                    return false;
                }
                break;
            }
            case JSON_NUM_STATE_EXP_SIGN: {
                JP_ATTR_FALLTHROUGH;
            }
            case JSON_NUM_STATE_EXP_DIGIT:
                if (ch >= '0' && ch <= '9') {
                    state = JSON_NUM_STATE_EXP_DIGIT;
                } else {
                    return false;
                }
                break;
        }
    }
    return state == JSON_NUM_STATE_ZERO || state == JSON_NUM_STATE_DIGIT || state == JSON_NUM_STATE_FRACTION ||
           state == JSON_NUM_STATE_EXP_DIGIT;
}

static bool is_json_string_literal(const unsigned char* str, size_t len) {
    if (len == 0) {
        return true;
    }
    const unsigned char ch = str[0];
    if (ch == 't' && len == 4) {
        return memcmp(str, "true", 4);
    }
    if (ch == 'f' && len == 5) {
        return memcmp(str, "false", 5);
    }
    if (ch == 'n' && len == 4) {
        return memcmp(str, "null", 4);
    }
    if (ch == '-' || (ch >= '0' && ch <= '9')) {
        return !is_json_numeric_literal(str, len);
    }
    return true;
}

unsigned char* jp_json_prefix_encoder(const jp_field_set_t* field_set, size_t* dst_len) {
    const char* msg_key     = "\"msg\":";
    size_t prefix_size      = 64;
    size_t json_ptr         = 0;
    unsigned char* metadata = NULL;

    for (size_t i = 0; i < field_set->len; i++) {
        prefix_size += field_set->fields[i]->key_len + field_set->fields[i]->val_len * JP_JSON_ESCAPE_MUL + 6;
    }

    metadata             = jp_mem_malloc(sizeof(unsigned char) * prefix_size);
    metadata[json_ptr++] = '{';
    for (size_t i = 0; i < field_set->len; i++) {
        const jp_field_t* field = field_set->fields[i];
        const bool is_json_str  = is_json_string_literal(field->val, field->val_len);

        metadata[json_ptr++] = '\"';
        memcpy(metadata + json_ptr, field->key, field->key_len);
        json_ptr             += field->key_len;
        metadata[json_ptr++]  = '\"';
        metadata[json_ptr++]  = ':';
        if (is_json_str) {
            metadata[json_ptr++] = '\"';
        }
        const size_t esc_size =
            jp_json_value_encoder(field->val, field->val_len, metadata + json_ptr, field->val_len * 6);
        json_ptr += esc_size;
        if (is_json_str) {
            metadata[json_ptr++] = '\"';
        }
        metadata[json_ptr++] = ',';
    }
    memcpy((void*) (metadata + json_ptr), msg_key, 6);
    json_ptr           += 6;
    metadata[json_ptr]  = '\0';
    *dst_len            = json_ptr;
    return metadata;
}

JP_ATTR_HOT
JP_ATTR_LEAF
size_t jp_json_value_encoder(const unsigned char* restrict src,
                             size_t src_len,
                             unsigned char* restrict dst,
                             size_t dst_len) {
    size_t src_ptr = 0, dst_ptr = 0;
    unsigned char ch, esc_len;

    while (src_ptr < src_len) {
        ch      = src[src_ptr++];
        esc_len = json_size_lut[ch];

        if (JP_ATTR_UNLIKELY(dst_ptr + esc_len > dst_len)) {
            break;
        }
        if (JP_ATTR_LIKELY(esc_len == 1)) {
            dst[dst_ptr++] = ch;
            continue;
        }
        if (esc_len == 2) {
            dst[dst_ptr++] = '\\';
            dst[dst_ptr++] = json_short_lookup[ch];
            continue;
        }
        dst[dst_ptr++] = '\\';
        dst[dst_ptr++] = 'u';
        dst[dst_ptr++] = '0';
        dst[dst_ptr++] = '0';
        dst[dst_ptr++] = hex_chars[ch >> 4];
        dst[dst_ptr++] = hex_chars[ch & 0x0F];
    }

    return dst_ptr;
}

unsigned char* jp_json_postfix_encoder(JP_ATTR_UNUSED const jp_field_set_t* field_set, size_t* dst_len) {
    *dst_len = 3;
    return jp_mem_strdup("\"}\n");
}