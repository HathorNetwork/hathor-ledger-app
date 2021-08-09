#pragma once

#include <stdint.h>   // uint*_t
#include <stdbool.h>  // bool

#include "../common/buffer.h"
#include "types.h"

/**
 * Handler for SIGN_TX command.
 *
 * We receive the serialized data to sign, and ask the user to confirm each output
 * When all are confirmed, we receive paths to derive and sign the data, returning the signature
 *
 * There are 3 stages:
 * - receiving data : p1=0
 * - signing data : p1=1
 * - caller is finished : p1=2
 *
 * Obs: The user should be able to call the process off on any stage, returning to the menu
 * and if the caller tries to bypass any stage or the user confirmation, the process should cancel.
 *
 * @param[in] cdata
 *   Buffer with data from caller.
 *
 * @param[in] stage
 *   For which stage the caller is sending the data.
 *
 * @param[in] chunk
 *   For receive data stage only, which data chunk index.
 *
 * @return zero or positive integer if success, negative integer otherwise.
 *
 */
int handler_sign_tx(buffer_t *cdata, sing_tx_stage_e stage, uint8_t chunk);
