#pragma once

#include <stddef.h>  // size_t
#include <stdint.h>  // uint*_t

#define TOKEN_UID_LEN       32
#define TOKEN_SYMBOL_LEN    20
#define TOKEN_NAME_LEN      120
#define TOKEN_SIGNATURE_LEN 100

typedef uint8_t token_uid[TOKEN_UID_LEN];

typedef struct {
    token_uid uid;
    uint8_t symbol_len;
    uint8_t symbol[TOKEN_SYMBOL_LEN];
    uint8_t name_len;
    uint8_t name[TOKEN_NAME_LEN];
    uint8_t version;
} token_t;
