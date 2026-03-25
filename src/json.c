#include <jp_json.h>

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

size_t jp_json_escape(const unsigned char* restrict src, size_t src_len, unsigned char* restrict dst, size_t dst_len) {
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