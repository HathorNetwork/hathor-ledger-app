#pragma once

#include <stddef.h>   // size_t
#include <stdint.h>   // uint*_t
#include <stdbool.h>  // bool

/**
 * Maximum derivation index to use.
 * 2 power 20 or 0x100000u
 * This number must never be higher than (1 << 31) because this is hardened index space
 */
#define MAX_DERIVATION_INDEX (1 << 20)

/**
 * Maximum length of BIP32 path allowed.
 *
 * m/purpose'/coin'/acct'/change/index
 * we don't need derivation past index level
 */
#define MAX_BIP32_PATH 5

typedef struct bip32_path {
    uint8_t length;
    uint32_t path[MAX_BIP32_PATH];
} bip32_path_t;

/**
 * Read BIP32 path from byte buffer.
 *
 * @param[in]  in
 *   Pointer to input byte buffer.
 * @param[in]  in_len
 *   Length of input byte buffer.
 * @param[out] out
 *   Pointer to output path.
 *
 * @return true if success, false otherwise.
 *
 */
bool bip32_path_read(const uint8_t *in, size_t in_len, bip32_path_t *out);

/**
 * Format BIP32 path as string.
 *
 * @param[in]  bip32_path
 *   Pointer to 32-bit integer input buffer.
 * @param[in]  bip32_path_len
 *   Maximum number of BIP32 paths in the input buffer.
 * @param[out] out string
 *   Pointer to output string.
 * @param[in]  out_len
 *   Length of the output string.
 *
 * @return true if success, false otherwise.
 *
 */
bool bip32_path_format(const uint32_t *bip32_path,
                       size_t bip32_path_len,
                       char *out,
                       size_t out_len);
