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
 * Tx input length
 */
#define TX_INPUT_LEN 35

/**
 * Pubkey hash length
 */
#define PUBKEY_HASH_LEN 20

/**
 * Secret length
 */
#define SECRET_LEN 32

/**
 * Max custom tokens allowed on a transaction
 */
#define TX_MAX_TOKENS 10

/**
 * Token data mask for authority output
 * If the first bit is set the output is an authority output
 */
#define TOKEN_DATA_AUTHORITY_MASK 0x80u
/**
 * Token data mask for token index
 *
 * index 0 means the token is HTR
 * Any other means that index-1 is the index of token on the token array
 */
#define TOKEN_DATA_INDEX_MASK 0x7Fu

#define MAX_NFT_DATA_LEN 100
