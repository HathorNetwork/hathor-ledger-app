#include "signature.h"

#include "types.h"
#include "../common/bip32.h"
#include "hathor.h"

#include "cx.h"

void gen_salt(uint32_t i, uint8_t *token_uid, uint8_t *out) {
    uint8_t message[36] = {0};  // 32 bytes from uid + 4 bytes from i

    memmove(message, token_uid, TOKEN_UID_LEN);
    // i in big endian
    message[TOKEN_UID_LEN + 0] = (uint8_t)(i >> 24);
    message[TOKEN_UID_LEN + 1] = (uint8_t)(i >> 16);
    message[TOKEN_UID_LEN + 2] = (uint8_t)(i >> 8);
    message[TOKEN_UID_LEN + 3] = (uint8_t)(i >> 0);

    cx_hash_sha256(message, TOKEN_UID_LEN + 4, out, 32);
}

void gen_signer_path(uint32_t secret, uint32_t *path, size_t *length) {
    *length = 5;
    path[0] = 44 | 0x80000000u;   // 44'
    path[1] = 280 | 0x80000000u;  // 280'
    path[2] = 0x80000000u;        // 0'
    path[3] = 0x80000000u;        // 0'
    path[4] = secret;             // secret
}

void init_token_signature_message(uint32_t secret, token_t *token, uint8_t *out) {
    // message == uid + symbol + name + version + salt
    uint8_t message[TOKEN_UID_LEN + TOKEN_SYMBOL_LEN + TOKEN_NAME_LEN + 1 + 32];
    uint8_t salt[32];

    size_t offset = 0;
    // add token uid
    memmove(message, token->uid, TOKEN_UID_LEN);
    offset += TOKEN_UID_LEN;
    // add token symbol
    memmove(message + offset, token->symbol, token->symbol_len);
    offset += token->symbol_len;
    // add token name
    memmove(message + offset, token->name, token->name_len);
    offset += token->name_len;
    // add token version
    message[offset] = token->version;
    offset += 1;
    // add salt
    gen_salt(secret, (uint8_t *) token->uid, salt);
    memmove(message + offset, salt, 32);
    offset += 32;
    // hash message with sha256
    cx_hash_sha256(message, offset, out, 32);
}

// sign token, return 0 if failed, signature length if ok
uint32_t sign_token(uint32_t secret,
                    token_t *token,
                    uint8_t *signature,
                    size_t sig_len,
                    size_t *out_len) {
    uint8_t hash[32], chain_code[32];
    cx_ecfp_private_key_t private_key = {0};

    uint32_t path[5] = {0};
    uint32_t info = 0;
    size_t path_len;
    size_t _sig_len = sig_len;

    init_token_signature_message(secret, token, hash);

    // init private_key
    gen_signer_path(secret, path, &path_len);

    derive_private_key(&private_key, chain_code, path, path_len);

    uint32_t err = cx_ecdsa_sign_no_throw(&private_key,
                                          CX_RND_RFC6979,
                                          CX_SHA256,
                                          hash,
                                          32,
                                          signature,
                                          &_sig_len,
                                          &info);
    // clean private_key
    explicit_bzero(&private_key, sizeof(private_key));
    explicit_bzero(&chain_code, sizeof(chain_code));
    *out_len = _sig_len;
    return err;
}

// verify token signature
bool verify_token_signature(uint32_t secret, token_t *token, uint8_t *signature, size_t sig_len) {
    bool sig_ok = false;
    uint8_t hash[32], chain_code[32];
    cx_ecfp_private_key_t private_key = {0};
    cx_ecfp_public_key_t public_key = {0};

    uint32_t path[5] = {0};
    size_t path_len;

    init_token_signature_message(secret, token, hash);

    // init private_key
    gen_signer_path(secret, path, &path_len);

    derive_private_key(&private_key, chain_code, path, path_len);
    init_public_key(&private_key, &public_key);

    sig_ok = cx_ecdsa_verify_no_throw(&public_key, hash, 32, signature, sig_len);

    // clean private_key
    explicit_bzero(&private_key, sizeof(private_key));
    explicit_bzero(&public_key, sizeof(public_key));
    explicit_bzero(&chain_code, sizeof(chain_code));

    return sig_ok;
}
