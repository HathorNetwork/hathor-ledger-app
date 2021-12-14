#pragma once

#include <stdint.h>   // uint*_t
#include <stddef.h>   // size_t
#include <stdbool.h>  // bool

#include "types.h"

/**
 * Sign the token information
 *
 * @param[in]  secret
 *   Internal secret to generate the salt and bip32 path.
 * @param[in]  token
 *   Pointer to token information to be signed.
 * @param[out] signature
 *   Pointer to signature byte buffer.
 * @param[in]  sig_len
 *   Signature byte buffer size.
 * @param[out] out_len
 *   Signature length.
 *
 * @return error code if any error occurs (0 means no error)
 *
 */
uint32_t sign_token(uint32_t secret,
                    token_t *token,
                    uint8_t *signature,
                    size_t sig_len,
                    size_t *out_len);

/**
 * Verify a token signature
 *
 * @param[in]  secret
 *   Internal secret to generate the salt and bip32 path.
 * @param[in]  token
 *   Pointer to token information to be signed.
 * @param[in] signature
 *   Pointer to signature byte buffer.
 * @param[in]  sig_len
 *   Signature byte buffer size.
 *
 * @return bool if signature is valid of not.
 *
 */
bool verify_token_signature(uint32_t secret, token_t *token, uint8_t *signature, size_t sig_len);
