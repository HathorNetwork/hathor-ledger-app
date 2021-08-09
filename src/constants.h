#pragma once

/**
 * Instruction class of the Hathor application.
 */
#define CLA 0xE0

/**
 * Maximum length of MAJOR_VERSION || MINOR_VERSION || PATCH_VERSION.
 */
#define APPVERSION_LEN 3

/**
 * Address length
 */
#define ADDRESS_LEN 25

/**
 * B58 encoded address length
 */
#define B58_ADDRESS_LEN 35

/**
 * Token UID length
 */
#define TOKEN_UID_LEN 32

/**
 * Tx input length
 */
#define TX_INPUT_LEN 35

/**
 * Pubkey hash length
 */
#define PUBKEY_HASH_LEN 20

/**
 * Maximum number of outputs on a transaction.
 */
#define MAX_NUM_TX_OUTPUTS 255