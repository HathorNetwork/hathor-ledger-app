#pragma once

#include <stddef.h>  // size_t
#include <stdint.h>  // uint*_t

#include "cx.h"

#include "constants.h"
#include "token/types.h"
#include "transaction/types.h"
#include "common/bip32.h"

/**
 * Enumeration for the status of IO.
 */
typedef enum {
    READY,     /// ready for new event
    RECEIVED,  /// data received
    WAITING    /// waiting
} io_state_e;

/**
 * Enumeration with expected INS of APDU commands.
 */
typedef enum {
    GET_VERSION = 0x03,             /// version of the application
    GET_ADDRESS = 0x04,             /// get address from BIP32 path
    GET_XPUB = 0x05,                /// XPUB of corresponding BIP32 path
    SIGN_TX = 0x06,                 /// sign transaction with BIP32 path
    SIGN_TOKEN_DATA = 0x07,         /// sign token data
    SEND_TOKEN_DATA = 0x08,         /// send token data
    VERIFY_TOKEN_SIGNATURE = 0x09,  /// verify token signature
    RESET_TOKEN_SIGNATURES = 0x0a,  /// invalidate all token signatures
    SEND_CREATE_TOKEN_DATA = 0x0b,  /// send create token data
} command_e;

/**
 * Structure with fields of APDU command.
 */
typedef struct {
    uint8_t cla;    /// Instruction class
    command_e ins;  /// Instruction code
    uint8_t p1;     /// Instruction parameter 1
    uint8_t p2;     /// Instruction parameter 2
    uint8_t lc;     /// Lenght of command data
    uint8_t* data;  /// Command data
} command_t;

/**
 * Enumeration with parsing state.
 */
typedef enum {
    STATE_NONE,       /// No state
    STATE_RECV_DATA,  /// Receiving data (for SIGN_TX)
    STATE_PARSED,     /// Transaction data parsed
    STATE_APPROVED    /// Transaction data approved
} state_e;

/**
 * Enumeration with user request type.
 */
typedef enum {
    CONFIRM_ADDRESS,     /// confirm address derived from public key
    CONFIRM_XPUB,        /// confirm access to xpub
    CONFIRM_TRANSACTION  /// confirm transaction information
} request_type_e;

#define MAX_SCREEN_LENGTH 12
#define CHAINCODE_LEN     32
#define FINGERPRINT_LEN   4
#define RAW_PUBKEY_LEN    65
/**
 * Structure for public key context information.
 */
typedef struct {
    bip32_path_t key_path;

    uint8_t raw_public_key[RAW_PUBKEY_LEN];
    uint8_t chain_code[CHAINCODE_LEN];
    uint8_t fingerprint[FINGERPRINT_LEN];
} xpub_ctx_t;

/**
 * Structure for change output information.
 */
typedef struct {
    uint8_t index;
    bip32_path_t path;
} change_output_info_t;

/**
 * Structure for transaction information context.
 */
typedef struct {
    uint8_t buffer[300];
    size_t buffer_len;
    // sha256 context for the hash
    cx_sha256_t sha256;
    uint8_t sighash_all[32];

    uint8_t change_len;
    change_output_info_t change_info[1 + TX_MAX_TOKENS];  // TX_MAX_TOKENS + HTR
    uint8_t change_output_index;
    bip32_path_t change_bip32_path;
    // tx
    uint16_t tx_version;
    uint8_t remaining_tokens;
    uint8_t remaining_inputs;
    uint8_t outputs_len;

    // index of the current output being decoded
    uint8_t current_output;
    // index of the output we should display, relative to outputs buffer
    // Once this reaches buffer_output_len, we should request more data
    uint8_t display_index;
    // number of outputs that are confirmed by the user + change outputs confirmed by the ledger
    // once this reaches outputs_len, we should ask confirmation to sign the transaction
    // This is the only index relative to outputs_len
    uint8_t confirmed_outputs;
    // number of outputs that we have decoded on the buffer, waiting for approval
    uint8_t buffer_output_len;
    tx_output_t outputs[10];  // max num of outputs on a call is 7
    tx_output_t decoded_output;
    // tokens is an array of ptrs to save memory since these tokens are already on a global ctx
    token_symbol_t* tokens[TX_MAX_TOKENS];
    uint8_t tokens_len;
} sign_tx_ctx_t;

typedef struct {
    uint8_t len;
    token_symbol_t tokens[TX_MAX_TOKENS];
} token_data_ctx_t;

/**
 * Structure for global context.
 */
typedef struct {
    state_e state;  /// state of the context
    union {
        xpub_ctx_t pk_info;     /// public key context
        sign_tx_ctx_t tx_info;  /// transaction context
    };
    request_type_e req_type;  /// user request
    bip32_path_t bip32_path;
    token_t token;
    uint8_t nft_data_len;
    uint8_t nft_data[MAX_NFT_DATA_LEN];
} global_ctx_t;

/**
 * Internal storage structure
 */

typedef struct internalStorage_t {
    uint8_t secret[SECRET_LEN];
} internalStorage_t;
