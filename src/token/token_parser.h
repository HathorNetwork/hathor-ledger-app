#pragma once

#include "types.h"
#include "../common/buffer.h"

bool parse_token(buffer_t *buf, token_t *token);

/**
 * Find the index of a token on the token registry
 *
 * The token registry is filled with calls to SEND_TOKEN_DATA
 *
 * @param[in] uid
 *   The uid of the token we want to find
 *
 * @return index of the token on registry or -1 if not on registry
 */
int8_t find_token_registry_index(uint8_t *uid);
