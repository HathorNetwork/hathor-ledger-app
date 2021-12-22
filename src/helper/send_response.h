#pragma once

#include "os.h"

#include "../common/macros.h"

/**
 * Length of public key.
 */
#define PUBKEY_LEN (MEMBER_SIZE(xpub_ctx_t, raw_public_key))
/**
 * Length of chain code.
 */
#define CHAINCODE_LEN (MEMBER_SIZE(xpub_ctx_t, chain_code))

/**
 * Senx xpub data from the public key derived by the app.
 */
int helper_send_response_xpub(void);

int helper_send_token_data_signature(void);
