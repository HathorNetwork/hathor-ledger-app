#pragma once

#include <stddef.h>  // size_t
#include <stdint.h>  // uint*_t

#include "../constants.h"

#define TOKEN_UID_LEN    32
#define TOKEN_SYMBOL_LEN 5   // ascii, 20 if utf-8
#define TOKEN_NAME_LEN   30  // ascii, 150 if utf-8
#define MESSAGE_LEN      SECRET_LEN + TOKEN_UID_LEN + TOKEN_SYMBOL_LEN + TOKEN_NAME_LEN + 1

typedef uint8_t token_uid[TOKEN_UID_LEN];

typedef struct {
    token_uid uid;
    uint8_t symbol_len;
    uint8_t symbol[TOKEN_SYMBOL_LEN];
    uint8_t name_len;
    uint8_t name[TOKEN_NAME_LEN];
    uint8_t version;
} token_t;
