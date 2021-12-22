#pragma once

#include "types.h"
#include "../common/buffer.h"

/**
 * Handler for VERIFY_TOKEN_SIGNATURES command.
 *
 * @return zero or positive integer if success, negative integer otherwise.
 *
 */
int handler_verify_token_signature(buffer_t *cdata);
