#include <stdint.h>  // uint*_t
#include <stddef.h>  // size_t

void write_u16_be(uint8_t *ptr, size_t offset, uint16_t value) {
    ptr[offset + 0] = (uint8_t)(value >> 8);
    ptr[offset + 1] = (uint8_t)(value >> 0);
}

void write_u32_be(uint8_t *ptr, size_t offset, uint32_t value) {
    ptr[offset + 0] = (uint8_t)(value >> 24);
    ptr[offset + 1] = (uint8_t)(value >> 16);
    ptr[offset + 2] = (uint8_t)(value >> 8);
    ptr[offset + 3] = (uint8_t)(value >> 0);
}

void write_u64_be(uint8_t *ptr, size_t offset, uint64_t value) {
    ptr[offset + 0] = (uint8_t)(value >> 56);
    ptr[offset + 1] = (uint8_t)(value >> 48);
    ptr[offset + 2] = (uint8_t)(value >> 40);
    ptr[offset + 3] = (uint8_t)(value >> 32);
    ptr[offset + 4] = (uint8_t)(value >> 24);
    ptr[offset + 5] = (uint8_t)(value >> 16);
    ptr[offset + 6] = (uint8_t)(value >> 8);
    ptr[offset + 7] = (uint8_t)(value >> 0);
}

void write_u16_le(uint8_t *ptr, size_t offset, uint16_t value) {
    ptr[offset + 0] = (uint8_t)(value >> 0);
    ptr[offset + 1] = (uint8_t)(value >> 8);
}

void write_u32_le(uint8_t *ptr, size_t offset, uint32_t value) {
    ptr[offset + 0] = (uint8_t)(value >> 0);
    ptr[offset + 1] = (uint8_t)(value >> 8);
    ptr[offset + 2] = (uint8_t)(value >> 16);
    ptr[offset + 3] = (uint8_t)(value >> 24);
}

void write_u64_le(uint8_t *ptr, size_t offset, uint64_t value) {
    ptr[offset + 0] = (uint8_t)(value >> 0);
    ptr[offset + 1] = (uint8_t)(value >> 8);
    ptr[offset + 2] = (uint8_t)(value >> 16);
    ptr[offset + 3] = (uint8_t)(value >> 24);
    ptr[offset + 4] = (uint8_t)(value >> 32);
    ptr[offset + 5] = (uint8_t)(value >> 40);
    ptr[offset + 6] = (uint8_t)(value >> 48);
    ptr[offset + 7] = (uint8_t)(value >> 56);
}
