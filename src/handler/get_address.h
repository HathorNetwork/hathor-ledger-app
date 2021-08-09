#pragma once

#include "../common/buffer.h"

/**
 * Handler for CONFIRM_ADDRESS command.
 *
 * We receive the desired bip32 path
 * we derive and show the user the address to confirm.
 *
 * @param[in] cdata
 *   Buffer with data from caller.
 *
 * @return zero or positive integer if success, negative integer otherwise.
 *
 */
int handler_get_address(buffer_t *cdata);
