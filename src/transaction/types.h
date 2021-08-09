#pragma once

#include <stddef.h>  // size_t
#include <stdint.h>  // uint*_t

#include "opcodes.h"

#include "../common/bip32.h"
#include "../constants.h"

typedef enum {
    TX_STATE_ERR = 1,
    TX_STATE_READY = 2,
} tx_decoder_state_e;

typedef enum {
    ELEM_TOKEN_UID,
    ELEM_INPUT,
    ELEM_OUTPUT,
} tx_decoder_elem_t;

typedef enum {
    SIGN_TX_STAGE_DATA = 0,
    SIGN_TX_STAGE_SIGN = 1,
    SIGN_TX_STAGE_DONE = 2
} sing_tx_stage_e;

/**
 * Structure for transaction output
 */
typedef struct {
    uint8_t index;
    uint64_t value;
    uint8_t token_data;
    uint8_t pubkey_hash[PUBKEY_HASH_LEN];  // hash160 of pubkey
} tx_output_t;
