#include <stdint.h>   // uint*_t
#include <stddef.h>   // size_t
#include <stdbool.h>  // bool
#include <string.h>   // memmove

#include "buffer.h"
#include "read.h"
#include "bip32.h"

bool buffer_can_read(const buffer_t *buffer, size_t n) {
    return buffer->size - buffer->offset >= n;
}

bool buffer_seek_set(buffer_t *buffer, size_t offset) {
    if (offset > buffer->size) {
        return false;
    }

    buffer->offset = offset;

    return true;
}

bool buffer_seek_cur(buffer_t *buffer, size_t offset) {
    if (buffer->offset + offset < buffer->offset ||  // overflow
        buffer->offset + offset > buffer->size) {    // exceed buffer size
        return false;
    }

    buffer->offset += offset;

    return true;
}

bool buffer_seek_end(buffer_t *buffer, size_t offset) {
    if (offset > buffer->size) {
        return false;
    }

    buffer->offset = buffer->size - offset;

    return true;
}

bool buffer_read_u8(buffer_t *buffer, uint8_t *value) {
    if (!buffer_can_read(buffer, 1)) {
        *value = 0;

        return false;
    }

    *value = buffer->ptr[buffer->offset];
    buffer_seek_cur(buffer, 1);

    return true;
}

bool buffer_read_u16(buffer_t *buffer, uint16_t *value, endianness_t endianness) {
    if (!buffer_can_read(buffer, 2)) {
        *value = 0;

        return false;
    }

    *value = ((endianness == BE) ? read_u16_be(buffer->ptr, buffer->offset)
                                 : read_u16_le(buffer->ptr, buffer->offset));
    buffer_seek_cur(buffer, 2);

    return true;
}

bool buffer_read_u32(buffer_t *buffer, uint32_t *value, endianness_t endianness) {
    if (!buffer_can_read(buffer, 4)) {
        *value = 0;

        return false;
    }

    *value = ((endianness == BE) ? read_u32_be(buffer->ptr, buffer->offset)
                                 : read_u32_le(buffer->ptr, buffer->offset));

    buffer_seek_cur(buffer, 4);

    return true;
}

bool buffer_read_u64(buffer_t *buffer, uint64_t *value, endianness_t endianness) {
    if (!buffer_can_read(buffer, 8)) {
        *value = 0;

        return false;
    }

    *value = ((endianness == BE) ? read_u64_be(buffer->ptr, buffer->offset)
                                 : read_u64_le(buffer->ptr, buffer->offset));

    buffer_seek_cur(buffer, 8);

    return true;
}

bool buffer_read_bip32_path(buffer_t *buffer, bip32_path_t *out) {
    if (!bip32_path_read(buffer->ptr + buffer->offset,
                         buffer->size - buffer->offset,
                         out)) {
        return false;
    }

    // 1 byte of path_length + 4*path_length of data
    buffer_seek_cur(buffer, 1 + (sizeof(out->path[0]) * out->length));

    return true;
}

bool buffer_copy(const buffer_t *buffer, uint8_t *out, size_t out_len) {
    if (buffer->size - buffer->offset > out_len) {
        return false;
    }

    memmove(out, buffer->ptr + buffer->offset, buffer->size - buffer->offset);

    return true;
}

bool buffer_move(buffer_t *buffer, uint8_t *out, size_t out_len) {
    if (!buffer_copy(buffer, out, out_len)) {
        return false;
    }

    buffer_seek_cur(buffer, out_len);

    return true;
}
