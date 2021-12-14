#pragma once

#include "types.h"
#include "../common/buffer.h"

/**
 * Handler for SIGN_TOKEN_DATA command.
 *
 * @return zero or positive integer if success, negative integer otherwise.
 *
 */
int handler_sign_token_data(buffer_t *cdata);
