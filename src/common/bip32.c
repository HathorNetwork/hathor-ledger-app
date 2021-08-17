#include <stdio.h>    // snprintf
#include <string.h>   // memset, strlen
#include <stddef.h>   // size_t
#include <stdint.h>   // uint*_t
#include <stdbool.h>  // bool

#include "bip32.h"
#include "read.h"

bool bip32_path_read(const uint8_t *in, size_t in_len, bip32_path_t *out) {
    if (in_len < 1 || in[0] > MAX_BIP32_PATH || in[0] * 4 + 1 > in_len) {
        return false;
    }
    size_t offset = 0;
    out->length = in[0];
    offset++;
    for (size_t i = 0; i < out->length; i++) {
        if (offset > in_len) {
            // shouldn't happen, we check length on the if above
            return false;
        }
        out->path[i] = read_u32_be(in, offset);
        offset += 4;
    }
    if (out->length > 4) {
        if (out->path[4] > MAX_DERIVATION_INDEX) {
            // we will not allow derivations past MAX_DERIVATION_INDEX
            // only on the index level
            return false;
        }
    }
    if (out->length > 3) {
        if ((out->path[3]) > 1) {
            // change level only allow 0 or 1
            return false;
        }
    }
    // BOLOS already verifies the first 3 levels, so we don't have to do it here.

    return true;
}

bool bip32_path_format(const uint32_t *bip32_path,
                       size_t bip32_path_len,
                       char *out,
                       size_t out_len) {
    if (bip32_path_len == 0 || bip32_path_len > MAX_BIP32_PATH) {
        return false;
    }

    size_t offset = 0;

    for (uint16_t i = 0; i < bip32_path_len; i++) {
        size_t written;

        snprintf(out + offset, out_len - offset, "%d", bip32_path[i] & 0x7FFFFFFFu);
        written = strlen(out + offset);
        if (written == 0 || written >= out_len - offset) {
            memset(out, 0, out_len);
            return false;
        }
        offset += written;

        if ((bip32_path[i] & 0x80000000u) != 0) {
            snprintf(out + offset, out_len - offset, "'");
            written = strlen(out + offset);
            if (written == 0 || written >= out_len - offset) {
                memset(out, 0, out_len);
                return false;
            }
            offset += written;
        }

        if (i != bip32_path_len - 1) {
            snprintf(out + offset, out_len - offset, "/");
            written = strlen(out + offset);
            if (written == 0 || written >= out_len - offset) {
                memset(out, 0, out_len);
                return false;
            }
            offset += written;
        }
    }

    return true;
}
