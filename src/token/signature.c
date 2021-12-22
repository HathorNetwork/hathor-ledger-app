#include "signature.h"

#include "types.h"

#include "cx.h"

void init_token_signature_message(uint8_t* secret, token_t *token, uint8_t *out) {
    // message == uid + symbol + name + version + salt
    size_t offset = 0;
    // add secret
    memmove(out, secret, SECRET_LEN);
    offset += SECRET_LEN;
    // add token uid
    memmove(out, token->uid, TOKEN_UID_LEN);
    offset += TOKEN_UID_LEN;
    // add token symbol
    memmove(out + offset, token->symbol, token->symbol_len);
    offset += token->symbol_len;
    // add token name
    memmove(out + offset, token->name, token->name_len);
    offset += token->name_len;
    // add token version
    out[offset] = token->version;
    offset += 1;
}

// sign token
void sign_token(uint8_t* secret, token_t *token, uint8_t *signature) {
    uint8_t message[MESSAGE_LEN];
    init_token_signature_message(secret, token, message);
    cx_hash_sha256(message, MESSAGE_LEN, signature, 32);
}

// verify token signature
bool verify_token_signature(uint8_t* secret, token_t *token, uint8_t *signature) {
    uint8_t message[MESSAGE_LEN];
    uint8_t sign[32];

    init_token_signature_message(secret, token, message);
    cx_hash_sha256(message, MESSAGE_LEN, sign, 32);
    return memcmp(signature, sign, 32);
}
