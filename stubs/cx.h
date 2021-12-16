#ifndef STUBS_CX_H
#define STUBS_CX_H

#include "mock_engine.h"
#include "ox.h"

// cx flags
#define CX_FLAG
#define CX_LAST        (1 << 0)
#define CX_RND_TRNG    (2 << 9)
#define CX_RND_RFC6979 (3 << 9)

#define CX_SHA256_SIZE 32

typedef uint32_t cx_err_t;

// keypairs
typedef struct {
    // cx_curve_t curve;
    size_t W_len;
    uint8_t W[65];
} cx_ecfp_public_key_t;
typedef struct {
    // cx_curve_t curve;
    size_t d_len;
    uint8_t d[32];
} cx_ecfp_private_key_t;

/** Message Digest algorithm identifiers. */
typedef enum {
    CX_NONE = 0,
    CX_RIPEMD160 = 1,  // xx bytes
    CX_SHA256 = 3,     // 32 bytes
} cx_md_t;

typedef struct {
    // struct cx_hash_header_s header;
    size_t blen;
    uint8_t block[64];
    uint8_t acc[8 * 4];
} cx_sha256_t;

typedef struct cx_ripemd160_s {
    // struct cx_hash_header_s header;
    size_t blen;
    uint8_t block[64];
    uint8_t acc[5 * 4];
} cx_ripemd160_t;

// MOCK: cx_hash_t
typedef struct {
    cx_md_t md_type;
    uint8_t buf[32];
} cx_hash_t;

MOCK_BOLOS_FN(cx_sha256_init, { cx_sha256_t out; })
int cx_sha256_init(cx_sha256_t *hash);
MOCK_BOLOS_FN(cx_ripemd160_init, { cx_ripemd160_t out; })
int cx_ripemd160_init(cx_ripemd160_t *hash);

MOCK_BOLOS_FN(cx_hash, {
    unsigned char *out;
    int len;
})
int cx_hash(cx_hash_t *hash,
            int mode,
            const unsigned char *in,
            unsigned int len,
            unsigned char *out,
            unsigned int out_len);

MOCK_BOLOS_FN(cx_ecfp_generate_pair, {
    cx_ecfp_public_key_t pubkey;
    cx_ecfp_private_key_t privkey;
})
int cx_ecfp_generate_pair(cx_curve_t curve,
                          cx_ecfp_public_key_t *pubkey,
                          cx_ecfp_private_key_t *privkey,
                          int keepprivate);

MOCK_BOLOS_FN(cx_ecfp_init_private_key, {
    cx_ecfp_private_key_t privkey;
    int len;
})
int cx_ecfp_init_private_key(cx_curve_t curve,
                             const unsigned char *rawkey,
                             unsigned int key_len,
                             cx_ecfp_private_key_t *pvkey);

MOCK_BOLOS_FN(cx_hash_sha256, {
    uint8_t *out;
    size_t len;
})
size_t cx_hash_sha256(const uint8_t *in, size_t len, uint8_t *out, size_t out_len);

MOCK_BOLOS_FN(cx_ecdsa_sign_no_throw, {
    uint8_t *sig;
    size_t sig_len;
    uint32_t info;
    uint32_t err;
})
cx_err_t cx_ecdsa_sign_no_throw(const cx_ecfp_private_key_t *pvkey,
                                uint32_t mode,
                                cx_md_t hashID,
                                const uint8_t *hash,
                                size_t hash_len,
                                uint8_t *sig,
                                size_t *sig_len,
                                uint32_t *info);

MOCK_BOLOS_FN(cx_ecdsa_verify_no_throw, { bool ok; })
bool cx_ecdsa_verify_no_throw(const cx_ecfp_public_key_t *pukey,
                              const uint8_t *hash,
                              size_t hash_len,
                              const uint8_t *sig,
                              size_t sig_len);

#endif  // STUBS_CX_H
