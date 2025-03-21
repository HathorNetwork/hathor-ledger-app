#pragma once

#include <stdint.h>
#include "../common/buffer.h"
#include "types.h"

#define ERR_MORE_DATA_REQUIRED 0xB100
#define ERR_UNSUPPORTED_SCRIPT 0xB101
#define ERR_INVALID_ARGS       0xB102
#define ERR_INVALID_SCRIPT     0xB103
#define ERR_DATA_LIMIT_REACHED 0xB104

/**
 * Identifies the given script type
 * @param[in] in
 *   Pointer to buffer with serialized script
 * @param[in] script_len
 *   Length of script
 *
 * @return script type
 */
script_type_t identify_script(buffer_t *in, uint16_t script_len);

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

/**
 * Read data from script and store it in `out`
 *
 * @param[in] in
 *   Pointer to buffer with serialized script
 * @param[in] script_len
 *   Length of script
 * @param[out] out
 *   Pointer to output structure.
 *
 * @return error code or 0 on success
 */
uint16_t read_data_script(buffer_t *in, uint16_t script_len, data_script_t *out);

/**
 * Identifies if the given script is P2PKH
 * `OP_DUP(1b) | OP_HASH160(1b) | PUBKEY_HASH_LEN(1b) | PUBKEY_HASH(20b) | OP_EQUALVERIFY(1b) |
 * OP_CHECKSIG(1b)`
 * @param[in] in
 *   Pointer to buffer with serialized script
 * @param[in] script_len
 *   Length of script
 *
 * @return boolean if the script is a P2PKH script.
 */
bool identify_p2pkh_script(buffer_t *in, uint16_t script_len);

/**
 * Identifies if the given script is P2SH
 * `OP_HASH160(1b) | SCRIPT_HASH_LEN(1b) | SCRIPT_HASH(20b) | OP_EQUAL(1b)`
 *
 * @param[in] in
 *   Pointer to buffer with serialized script
 * @param[in] script_len
 *   Length of script
 *
 * @return boolean if the script is a P2SH script.
 */
bool identify_p2sh_script(buffer_t *in, uint16_t script_len);

/**
 * Identifies if the given script is a data script
 * `DATA_LEN(1b) | DATA(var)`
 *   - Only valid for lengths [1, 75]
 * `OP_PUSHDATA1(1b) | DATA_LEN(1b) | DATA(var)`
 *   - Valid for lengths [1, 255] but usually only used for (75, 255]
 *
 * @param[in] in
 *   Pointer to buffer with serialized script
 * @param[in] script_len
 *   Length of script
 *
 * @return boolean if the script is a data script.
 */
bool identify_data_script(buffer_t *in, uint16_t script_len);
