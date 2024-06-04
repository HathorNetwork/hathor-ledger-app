#pragma once

#include "../common/buffer.h"

/**
 * Handler for SEND_CREATE_TOKEN_DATA command.
 *
 * @param token_type - token type
 * @param cdata - data
 * @return 0 if success
 */
int handler_send_create_token_data(uint8_t token_type, buffer_t *cdata);
