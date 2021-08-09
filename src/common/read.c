#include <stdint.h>  // uint*_t
#include <stddef.h>  // size_t

uint16_t read_u16_be(const uint8_t *ptr, size_t offset) {
    return (uint16_t) ptr[offset + 0] << 8 |  //
           (uint16_t) ptr[offset + 1] << 0;
}

uint32_t read_u32_be(const uint8_t *ptr, size_t offset) {
    return (uint32_t) ptr[offset + 0] << 24 |  //
           (uint32_t) ptr[offset + 1] << 16 |  //
           (uint32_t) ptr[offset + 2] << 8 |   //
           (uint32_t) ptr[offset + 3] << 0;
}

uint64_t read_u64_be(const uint8_t *ptr, size_t offset) {
    return (uint64_t) ptr[offset + 0] << 56 |  //
           (uint64_t) ptr[offset + 1] << 48 |  //
           (uint64_t) ptr[offset + 2] << 40 |  //
           (uint64_t) ptr[offset + 3] << 32 |  //
           (uint64_t) ptr[offset + 4] << 24 |  //
           (uint64_t) ptr[offset + 5] << 16 |  //
           (uint64_t) ptr[offset + 6] << 8 |   //
           (uint64_t) ptr[offset + 7] << 0;
}

uint16_t read_u16_le(const uint8_t *ptr, size_t offset) {
    return (uint16_t) ptr[offset + 0] << 0 |  //
           (uint16_t) ptr[offset + 1] << 8;
}

uint32_t read_u32_le(const uint8_t *ptr, size_t offset) {
    return (uint32_t) ptr[offset + 0] << 0 |   //
           (uint32_t) ptr[offset + 1] << 8 |   //
           (uint32_t) ptr[offset + 2] << 16 |  //
           (uint32_t) ptr[offset + 3] << 24;
}

uint64_t read_u64_le(const uint8_t *ptr, size_t offset) {
    return (uint64_t) ptr[offset + 0] << 0 |   //
           (uint64_t) ptr[offset + 1] << 8 |   //
           (uint64_t) ptr[offset + 2] << 16 |  //
           (uint64_t) ptr[offset + 3] << 24 |  //
           (uint64_t) ptr[offset + 4] << 32 |  //
           (uint64_t) ptr[offset + 5] << 40 |  //
           (uint64_t) ptr[offset + 6] << 48 |  //
           (uint64_t) ptr[offset + 7] << 56;
}
