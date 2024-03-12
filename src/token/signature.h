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
 * @param[in]  siglen
 *   Length of signature buffer.
 * @return 0 if success
 */
int sign_token(uint8_t secret[static SECRET_LEN], token_t *token, uint8_t *signature, size_t siglen);

/**
 * Verify a token signature
 *
 * @param[in]  secret
 *   Internal secret to generate the salt and bip32 path.
 * @param[in]  token
 *   Pointer to token information to be signed.
 * @param[in] signature
 *   Pointer to signature byte buffer.
 * @param[in]  siglen
 *   Length of signature buffer.
 *
 * @return int 0 if signature is valid, 1 if invalid, anything else is an error.
 *
 */
int verify_token_signature(uint8_t *secret, token_t *token, uint8_t *signature, size_t siglen);

/**
 * Verify a token signature from the raw apdu data
 *
 * @param[in]  cdata
 *   Pointer to raw apdu data buffer.
 * @param[out] token
 *   Token parsed.
 *
 * @return int 0 if signature is valid, 1 if invalid, anything else is an error.
 *
 *
 */
int check_token_signature_from_apdu(buffer_t *cdata, token_t *token);
