#include <stdint.h>   // uint*_t
#include <stddef.h>   // size_t
#include <stdbool.h>  // bool
#include <string.h>   // memmove

#include "os.h"

#include "hathor.h"
#include "constants.h"

// Path prefix of 44'/280'
const uint32_t htr_bip44[] = {44 | 0x80000000, HATHOR_BIP44_CODE | 0x80000000};

int sha256d(uint8_t *in, size_t inlen, uint8_t *out, size_t outlen) {
    cx_sha256_t hash;
    uint8_t buffer[32];
    cx_err_t error = CX_OK;

    // Check for NULL pointers
    if ((in == NULL) || (out == NULL) || (outlen < 32)) {
        return 1;
    }

    // sha256 of input to `buffer`
    cx_sha256_init(&hash);
    CX_CHECK(cx_hash_no_throw(&hash.header, CX_LAST, in, inlen, buffer, 32));
    // sha256 of buffer to `out`
    cx_sha256_init(&hash);
    CX_CHECK(cx_hash_no_throw(&hash.header, CX_LAST, buffer, 32, out, 32));

end:
    if (error != CX_OK) {
        return 2;
    }
    return 0;
}

int hash160(uint8_t *in, size_t inlen, uint8_t *out, size_t outlen) {
    union {
        cx_sha256_t shasha;
        cx_ripemd160_t riprip;
    } u;
    uint8_t buffer[32] = {0};
    cx_err_t error = CX_OK;

    if ((in == NULL) || (out == NULL) || (outlen < 20)) {
        return 1;
    }

    cx_sha256_init(&u.shasha);
    CX_CHECK(cx_hash_no_throw(&u.shasha.header, CX_LAST, in, inlen, buffer, 32));
    cx_ripemd160_init(&u.riprip);
    CX_CHECK(cx_hash_no_throw(&u.riprip.header, CX_LAST, buffer, 32, out, 20));

end:
    if (error != CX_OK) {
        return 2;
    }
    return error;
}

int compress_public_key(uint8_t *value, size_t len) {
    if ((value == NULL) || len < 65) {
        return 1;
    }
    value[0] = ((value[64] & 1) ? 0x03 : 0x02);
    return 0;
}

int address_from_hash(const uint8_t *hash,
                      const size_t hashlen,
                      const uint8_t version_byte,
                      uint8_t *out,
                      size_t outlen) {
    uint8_t buffer[32] = {0};
    if ((hash == NULL) || (out == NULL) || (hashlen < PUBKEY_HASH_LEN) || (outlen < 25)) {
        return 1;
    }
    // prepend version
    out[0] = version_byte;
    memmove(out + 1, hash, PUBKEY_HASH_LEN);
    // sha256d of above
    if (sha256d(out, 21, buffer, 32)) {
        return 1;
    };
    // grab first 4 bytes (checksum)
    memmove(out + 21, buffer, 4);
    return 0;
}

int address_from_script_hash(const uint8_t *script_hash,
                             const size_t script_hash_len,
                             uint8_t *out,
                             size_t outlen) {
    return address_from_hash(script_hash, script_hash_len, P2SH_VERSION_BYTE, out, outlen);
}

int address_from_pubkey_hash(const uint8_t *public_key_hash,
                             const size_t public_key_hash_len,
                             uint8_t *out,
                             size_t outlen) {
    return address_from_hash(public_key_hash, public_key_hash_len, P2PKH_VERSION_BYTE, out, outlen);
}

int address_from_pubkey(cx_ecfp_public_key_t *public_key, uint8_t *out, size_t outlen) {
    uint8_t buffer[PUBKEY_HASH_LEN] = {0};

    if ((public_key == NULL) || (out == NULL) || (outlen < 25)) {
        return 1;
    }

    // compress_public_key
    if (compress_public_key(public_key->W, sizeof(public_key->W) / sizeof(public_key->W[0]))) {
        return 1;
    }
    // hash160
    if (hash160(public_key->W, 33, buffer, PUBKEY_HASH_LEN)) {
        return 1;
    }
    // address_from_pubkey_hash
    return address_from_pubkey_hash(buffer, PUBKEY_HASH_LEN, out, outlen);
}

int derive_private_key(cx_ecfp_private_key_t *private_key,
                       uint8_t chain_code[static 32],
                       const uint32_t *bip32_path,
                       uint8_t bip32_path_depth) {
    cx_err_t error = CX_OK;
    uint8_t raw_privkey[65];

    if ((private_key == NULL) || (chain_code == NULL) || (bip32_path == NULL)) {
        return 1;
    }

    // derive private key
    CX_CHECK(os_derive_bip32_with_seed_no_throw(HDW_NORMAL,
                                                CX_CURVE_256K1,
                                                bip32_path,
                                                bip32_path_depth,
                                                raw_privkey,
                                                chain_code,
                                                NULL,
                                                0));

    CX_CHECK(cx_ecfp_init_private_key_no_throw(CX_CURVE_256K1, raw_privkey, 32, private_key));

end:
    explicit_bzero(raw_privkey, sizeof(raw_privkey));
    if (error != CX_OK) {
        explicit_bzero(private_key, sizeof(cx_ecfp_256_private_key_t));
        return error;
    }
    return 0;
}

int init_public_key(cx_ecfp_private_key_t *private_key, cx_ecfp_public_key_t *public_key) {
    if ((private_key == NULL) || (public_key == NULL)) {
        return 1;
    }
    // generate corresponding public key
    return cx_ecfp_generate_pair_no_throw(CX_CURVE_256K1, public_key, private_key, true);
}
