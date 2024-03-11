#include <stdint.h>   // uint*_t
#include <stddef.h>   // size_t
#include <stdbool.h>  // bool
#include <string.h>   // memmove

#include "os.h"

#include "hathor.h"
#include "constants.h"

// Path prefix of 44'/280'
const uint32_t htr_bip44[] = {44 | 0x80000000, HATHOR_BIP44_CODE | 0x80000000};

void sha256d(uint8_t *in, size_t inlen, uint8_t *out) {
    cx_sha256_t hash;
    uint8_t buffer[32];

    // Check for NULL pointers
    if ((in == NULL) || (out == NULL)) {
        return;
    }

    // sha256 of input to `buffer`
    cx_sha256_init(&hash);
    CX_THROW(cx_hash_no_throw(&hash.header, CX_LAST, in, inlen, buffer, 32));
    // sha256 of buffer to `out`
    cx_sha256_init(&hash);
    CX_THROW(cx_hash_no_throw(&hash.header, CX_LAST, buffer, 32, out, 32));
}

void hash160(uint8_t *in, size_t inlen, uint8_t *out) {
    union {
        cx_sha256_t shasha;
        cx_ripemd160_t riprip;
    } u;
    uint8_t buffer[32] = {0};

    if ((in == NULL) || (out == NULL)) {
        return;
    }

    cx_sha256_init(&u.shasha);
    CX_THROW(cx_hash_no_throw(&u.shasha.header, CX_LAST, in, inlen, buffer, 32));
    cx_ripemd160_init(&u.riprip);
    CX_THROW(cx_hash_no_throw(&u.riprip.header, CX_LAST, buffer, 32, out, 20));
}

void compress_public_key(uint8_t *value) {
    value[0] = ((value[64] & 1) ? 0x03 : 0x02);
}

void address_from_pubkey_hash(const uint8_t *public_key_hash, uint8_t *out) {
    uint8_t buffer[32] = {0};
    // prepend version
    out[0] = P2PKH_VERSION_BYTE;
    memmove(out + 1, public_key_hash, PUBKEY_HASH_LEN);
    // sha256d of above
    sha256d(out, 21, buffer);
    // grab first 4 bytes (checksum)
    memmove(out + 21, buffer, 4);
}

void address_from_pubkey(cx_ecfp_public_key_t *public_key, uint8_t *out) {
    uint8_t buffer[PUBKEY_HASH_LEN] = {0};

    if ((public_key == NULL) || (out == NULL)) {
        return;
    }
    // compress_public_key
    compress_public_key(public_key->W);
    // hash160
    hash160(public_key->W, 33, buffer);
    // address_from_pubkey_hash
    address_from_pubkey_hash(buffer, out);
}

void derive_private_key(cx_ecfp_private_key_t *private_key,
                        uint8_t chain_code[static 32],
                        const uint32_t *bip32_path,
                        uint8_t bip32_path_len) {
    cx_err_t error = CX_OK;
    uint8_t raw_privkey[65];

    CX_CHECK(os_derive_bip32_with_seed_no_throw(HDW_NORMAL,
                                                CX_CURVE_256K1,
                                                bip32_path,
                                                bip32_path_len,
                                                raw_privkey,
                                                chain_code,
                                                NULL,
                                                0));

    CX_CHECK(cx_ecfp_init_private_key_no_throw(CX_CURVE_256K1, raw_privkey, 32, private_key));

end:
    explicit_bzero(raw_privkey, sizeof(raw_privkey));
    if (error != CX_OK) {
        explicit_bzero(private_key, sizeof(cx_ecfp_256_private_key_t));
        THROW(error);
    }
}

void init_public_key(cx_ecfp_private_key_t *private_key, cx_ecfp_public_key_t *public_key) {
    // generate corresponding public key
    CX_THROW(cx_ecfp_generate_pair_no_throw(CX_CURVE_256K1, public_key, private_key, true));
}
