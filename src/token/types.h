#pragma once

#include <stddef.h>  // size_t
#include <stdint.h>  // uint*_t

#include "../constants.h"

#define TOKEN_UID_LEN        32
#define MAX_TOKEN_SYMBOL_LEN 5   // ascii, 20 if utf-8
#define MAX_TOKEN_NAME_LEN   30  // ascii, 120 if utf-8
#define MAX_MESSAGE_LEN      SECRET_LEN + TOKEN_UID_LEN + MAX_TOKEN_SYMBOL_LEN + MAX_TOKEN_NAME_LEN + 1

typedef uint8_t token_uid_t[TOKEN_UID_LEN];
typedef struct {
    token_uid_t uid;
    uint8_t symbol_len;
    uint8_t symbol[MAX_TOKEN_SYMBOL_LEN];
    uint8_t name_len;
    uint8_t name[MAX_TOKEN_NAME_LEN];
    uint8_t version;
} token_t;

typedef struct {
    token_uid_t uid;
    char symbol[MAX_TOKEN_SYMBOL_LEN + 1];
} token_symbol_t;

typedef enum { CUSTOM_TOKEN = 0x01, NFT_TOKEN = 0x02 } token_type_t;
