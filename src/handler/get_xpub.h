#pragma once

#include <stddef.h>   // size_t
#include <stdbool.h>  // bool
#include <stdint.h>   // uint*_t

#include "../types.h"
#include "../common/buffer.h"

/**
 * Handler for GET_XPUB command.
 * User confirms access and application sends the xpub.
 *
 * @param[in] cdata
 *   Buffer with data from caller.
 *
 * @return zero or positive integer if success, negative integer otherwise.
 *
 */

int handler_get_xpub(buffer_t *cdata);
