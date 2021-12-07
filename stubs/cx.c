#include "cx.h"

MOCK_BOLOS(cx_sha256_init)
int cx_sha256_init(cx_sha256_t *hash) {
    MOCK_FN_INTERNAL(cx_sha256_init);
    memmove(hash, &response->out, sizeof(cx_sha256_t));
    // return the id of the hash
    return CX_SHA256;
}

MOCK_BOLOS(cx_ripemd160_init)
int cx_ripemd160_init(cx_ripemd160_t *hash) {
    MOCK_FN_INTERNAL(cx_sha256_init);
    memmove(hash, &response->out, sizeof(cx_ripemd160_t));
    // return the id of the hash
    return CX_RIPEMD160;
}

MOCK_BOLOS(cx_hash)
int cx_hash(cx_hash_t *hash,
            int mode,
            const unsigned char *in,
            unsigned int len,
            unsigned char *out,
            unsigned int out_len) {
    MOCK_FN_INTERNAL(cx_hash);
    UNUSED(hash);
    UNUSED(mode);
    UNUSED(in);
    UNUSED(len);
    UNUSED(out_len);
    memmove(out, &response->out, response->len);
    return response->len;
}

MOCK_BOLOS(cx_ecfp_generate_pair)
int cx_ecfp_generate_pair(cx_curve_t curve,
                          cx_ecfp_public_key_t *pubkey,
                          cx_ecfp_private_key_t *privkey,
                          int keepprivate) {
    MOCK_FN_INTERNAL(cx_ecfp_generate_pair);
    UNUSED(keepprivate);
    UNUSED(curve);
    memmove(pubkey, &response->pubkey, sizeof(response->pubkey));
    memmove(privkey, &response->privkey, sizeof(response->privkey));
    return 0;
}

MOCK_BOLOS(cx_ecfp_init_private_key)
int cx_ecfp_init_private_key(cx_curve_t curve,
                             const unsigned char *rawkey,
                             unsigned int key_len,
                             cx_ecfp_private_key_t *pvkey) {
    MOCK_FN_INTERNAL(cx_ecfp_init_private_key);
    memmove(pvkey, &response->privkey, sizeof(response->privkey));
    return response->len;
}

MOCK_BOLOS(cx_hash_sha256)
size_t cx_hash_sha256(const uint8_t *in, size_t len, uint8_t *out, size_t out_len) {
    MOCK_FN_INTERNAL(cx_hash_sha256);
    memmove(out, response->out, response->len);
    return response->len;
}

MOCK_BOLOS(cx_ecdsa_sign_no_throw)
cx_err_t cx_ecdsa_sign_no_throw(const cx_ecfp_private_key_t *pvkey,
                                uint32_t mode,
                                cx_md_t hashID,
                                const uint8_t *hash,
                                size_t hash_len,
                                uint8_t *sig,
                                size_t *sig_len,
                                uint32_t *info) {
    MOCK_FN_INTERNAL(cx_ecdsa_sign_no_throw);
    memmove(sig, response->sig, response->sig_len);
    *sig_len = response->sig_len;
    *info = response->info;

    return response->err;
}

MOCK_BOLOS(cx_ecdsa_verify_no_throw)
bool cx_ecdsa_verify_no_throw(const cx_ecfp_public_key_t *pukey,
                              const uint8_t *hash,
                              size_t hash_len,
                              const uint8_t *sig,
                              size_t sig_len) {
    MOCK_FN_INTERNAL(cx_ecdsa_verify_no_throw);
    return response->ok;
}
