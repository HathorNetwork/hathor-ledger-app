#pragma once

#include "types.h"
#include "../common/buffer.h"

/**
 * Handler for SEND_TOKEN_DATA command.
 *
 * @return zero or positive integer if success, negative integer otherwise.
 *
 */
int handler_send_token_data(bool first, buffer_t *cdata);
