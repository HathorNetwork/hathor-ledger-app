#ifndef TX_SCRIPT_H
#define TX_SCRIPT_H

#define ERR_UNSUPPORTED_SCRIPT 0xB101
#define ERR_INVALID_ARGS 0xB102

#include <stdint.h>
#include "../common/buffer.h"
#include "types.h"

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
uint16_t parse_output_script(buffer_t *in, size_t script_len, output_script_info_t *out);

#endif  // TX_SCRIPT_H
