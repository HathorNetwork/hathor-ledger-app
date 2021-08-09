#include <stdint.h>   // uint*_t
#include <stddef.h>   // size_t
#include <stdbool.h>  // bool

#include "varint.h"
#include "write.h"
#include "read.h"

uint8_t varint_size(uint64_t value) {
    if (value <= 0xFC) {
        return 1;
    }

    if (value <= UINT16_MAX) {
        return 3;
    }

    if (value <= UINT32_MAX) {
        return 5;
    }

    return 9;  // <= UINT64_MAX
}

int varint_read(const uint8_t *in, size_t in_len, uint64_t *value) {
    if (in_len < 1) {
        return -1;
    }

    uint8_t prefix = in[0];

    if (prefix == 0xFD) {
        if (in_len < 3) {
            return -1;
        }
        *value = (uint64_t) read_u16_le(in, 1);
        return 3;
    }

    if (prefix == 0xFE) {
        if (in_len < 5) {
            return -1;
        }
        *value = (uint64_t) read_u32_le(in, 1);
        return 5;
    }

    if (prefix == 0xFF) {
        if (in_len < 9) {
            return -1;
        }
        *value = (uint64_t) read_u64_le(in, 1);
        return 9;
    }

    *value = (uint64_t) prefix;  // prefix <= 0xFC

    return 1;
}

int varint_write(uint8_t *out, size_t offset, uint64_t value) {
    uint8_t varint_len = varint_size(value);

    switch (varint_len) {
        case 1:
            out[offset] = (uint8_t) value;
            break;
        case 3:
            out[offset++] = 0xFD;
            write_u16_le(out, offset, (uint16_t) value);
            break;
        case 5:
            out[offset++] = 0xFE;
            write_u32_le(out, offset, (uint32_t) value);
            break;
        case 9:
            out[offset++] = 0xFF;
            write_u64_le(out, offset, (uint64_t) value);
            break;
        default:
            return -1;
    }

    return varint_len;
}
