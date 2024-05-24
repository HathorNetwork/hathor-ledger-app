#ifndef TRANSACTION_DESERIALIZE_H
#define TRANSACTION_DESERIALIZE_H

#include <stdint.h>  // uint*_t
#include <stddef.h>  // size_t
#include "types.h"   // tx_output_t

/**
 * Parse raw transaction output from buffer.
 *
 * @param[in] in
 *   Pointer to buffer with serialized transaction.
 * @param[in] inlen
 *   Size of buffer holding serialized transaction.
 * @param[out] output
 *   Pointer to output structure.
 *
 * @return size in bytes of the serialized output from `in`
 *
 */
size_t parse_output(uint8_t *in, size_t inlen, tx_output_t *output);

/**
 * Check if output is an authority output
 *
 * @param[in] token_data
 *   Token data
 *
 * @return true if output is an authority output
 */
bool is_authority_output(uint8_t token_data);

/**
 * Check if output is a mint authority
 *
 * @param[in] token_data
 *   Token data
 * @param[in] value
 *   Value
 * @return true if output is a mint authority
 */
bool is_mint_authority(uint8_t token_data, uint64_t value);

/**
 * Check if output is a melt authority
 *
 * @param[in] token_data
 *   Token data
 * @param[in] value
 *   Value
 * @return true if output is a melt authority
 */
bool is_melt_authority(uint8_t token_data, uint64_t value);

#endif
