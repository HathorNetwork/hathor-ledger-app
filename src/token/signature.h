#pragma once

#include <stdint.h>   // uint*_t
#include <stddef.h>   // size_t
#include <stdbool.h>  // bool

#include "../common/buffer.h"

#include "types.h"

/**
 * Sign the token information
 *
 * @param[in]  secret
 *   Internal secret.
 * @param[in]  token
 *   Pointer to token information to be signed.
 * @param[out] signature
 *   Pointer to signature byte buffer.
 */
void sign_token(uint8_t *secret, token_t *token, uint8_t *signature);

/**
 * Verify a token signature
 *
 * @param[in]  secret
 *   Internal secret to generate the salt and bip32 path.
 * @param[in]  token
 *   Pointer to token information to be signed.
 * @param[in] signature
 *   Pointer to signature byte buffer.
 *
 * @return bool if signature is valid of not.
 *
 */
bool verify_token_signature(uint8_t *secret, token_t *token, uint8_t *signature);

/**
 * Verify a token signature from the raw apdu data
 *
 * @param[in]  cdata
 *   Pointer to raw apdu data buffer.
 * @param[out] token
 *   Token parsed.
 *
 * @return bool if signature is valid of not.
 *
 */
bool check_token_signature_from_apdu(buffer_t *cdata, token_t *token);
