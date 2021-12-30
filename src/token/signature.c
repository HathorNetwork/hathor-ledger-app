#include "signature.h"

#include <string.h>  // memmove

#include "token_parser.h"
#include "../types.h"
#include "../storage.h"

#include "cx.h"

size_t init_token_signature_message(uint8_t *secret, token_t *token, uint8_t *out) {
    // message == secret + uid + symbol + name + version + salt
    size_t offset = 0;
    // add secret
    memmove(out, secret, SECRET_LEN);
    offset += SECRET_LEN;
    // add token uid
    memmove(out + offset, token->uid, TOKEN_UID_LEN);
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

    return offset;
}

// sign token
void sign_token(uint8_t *secret, token_t *token, uint8_t *signature) {
    uint8_t message[MAX_MESSAGE_LEN];
    size_t message_len = init_token_signature_message(secret, token, message);
    cx_hash_sha256(message, message_len, signature, 32);
}

// verify token signature
bool verify_token_signature(uint8_t *secret, token_t *token, uint8_t *signature) {
    uint8_t sign[32];
    sign_token(secret, token, sign);
    return memcmp(signature, sign, 32) == 0;
}

bool check_token_signature_from_apdu(buffer_t *cdata, token_t *token) {
    uint8_t signature[32];
    uint8_t secret[SECRET_LEN];
    // parse info on token
    if (!parse_token(cdata, token)) return false;

    // extract signature from cdata
    if (!buffer_read_bytes(cdata, signature, 32, 32)) return false;
    get_secret(secret);
    return verify_token_signature(secret, token, signature);
}
