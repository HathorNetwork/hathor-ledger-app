#pragma once

#include <stdint.h>
#include "../common/buffer.h"
#include "types.h"

#define ERR_MORE_DATA_REQUIRED 0xB100
#define ERR_UNSUPPORTED_SCRIPT 0xB101
#define ERR_INVALID_ARGS       0xB102

/**
 * Parse output script
 * @param[in] in
 *   Pointer to buffer with serialized script
 * @param[in] script_len
 *   Length of script
 * @param[out] out
 *   Pointer to output structure.
 *
 * @return error code or 0 on success
 */
uint16_t parse_output_script(buffer_t *in, uint16_t script_len, output_script_info_t *out);
