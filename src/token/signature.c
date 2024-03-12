#include "signature.h"

#include <string.h>  // memmove, explicit_bzero

#include "token_parser.h"
#include "../types.h"
#include "../storage.h"

#include "cx.h"
#include "types.h"

size_t init_token_signature_message(uint8_t secret[static SECRET_LEN],
                                    token_t *token,
                                    uint8_t *out,
                                    size_t outlen) {
    if (secret == NULL || token == NULL || out == NULL || outlen < MAX_MESSAGE_LEN) {
        return 0;
    }
    // message == secret + uid + symbol + name + version
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
int sign_token(uint8_t secret[static SECRET_LEN],
               token_t *token,
               uint8_t *signature,
               size_t siglen) {
    uint8_t message[MAX_MESSAGE_LEN];

    if (secret == NULL || token == NULL || signature == NULL || siglen < 32) {
        return 1;
    }

    size_t message_len = init_token_signature_message(secret, token, message, MAX_MESSAGE_LEN);
    if (!message_len) {
        return 2;
    }
    cx_hash_sha256(message, message_len, signature, 32);
    return 0;
}

// verify token signature
int verify_token_signature(uint8_t *secret, token_t *token, uint8_t *signature) {
    uint8_t sign[32];
    if (sign_token(secret, token, sign, 32)) {
        return 2;
    }
    if (memcmp(signature, sign, 32) == 0) {
        return 0;
    } else {
        return 1;
    }
}

int check_token_signature_from_apdu(buffer_t *cdata, token_t *token) {
    uint8_t signature[32];
    uint8_t secret[SECRET_LEN];

    if (cdata == NULL || token == NULL) {
        return 2;
    }

    // parse info on token
    if (!parse_token(cdata, token)) {
        return 3;
    };

    // extract signature from cdata
    if (!buffer_read_bytes(cdata, signature, 32, 32)) {
        return 4;
    }
    if (get_secret(secret)) {
        return 5;
    }
    int is_token_verified = verify_token_signature(secret, token, signature);

    // clear secret
    explicit_bzero(secret, SECRET_LEN);

    return is_token_verified;
}
