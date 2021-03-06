#include <stddef.h>  // size_t
#include <stdint.h>  // uint*_t
#include <string.h>  // memmove

#include "send_response.h"
#include "../constants.h"
#include "../globals.h"
#include "../sw.h"
#include "common/buffer.h"
#include "../hathor.h"
#include "../storage.h"

#include "../token/signature.h"

int helper_send_response_xpub() {
    uint8_t resp[PUBKEY_LEN + CHAINCODE_LEN + 4] = {0};
    size_t offset = 0;
    memmove(resp + offset, G_context.pk_info.raw_public_key, PUBKEY_LEN);
    offset += PUBKEY_LEN;
    memmove(resp + offset, G_context.pk_info.chain_code, CHAINCODE_LEN);
    offset += CHAINCODE_LEN;
    memmove(resp + offset, G_context.pk_info.fingerprint, 4);
    offset += 4;

    return io_send_response(&(const buffer_t){.ptr = resp, .size = offset, .offset = 0}, SW_OK);
}

int helper_send_token_data_signature() {
    uint8_t sign[32];
    uint8_t secret[SECRET_LEN];

    get_secret(secret);
    sign_token(secret, &G_context.token, sign);

    // return the signature
    return io_send_response(&(const buffer_t){.ptr = sign, .size = 32, .offset = 0}, SW_OK);
}
