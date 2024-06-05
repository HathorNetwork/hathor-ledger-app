#include <string.h>

#include "token_parser.h"

#include "../common/buffer.h"
#include "../common/format.h"
#include "../globals.h"

bool parse_token(buffer_t *buf, token_t *token) {
    if (buf == NULL || token == NULL) {
        return false;
    }
    // read uid, len(symbol), symbol, len(name), name, version
    if (!(buffer_read_u8(buf, &token->version) &&
          buffer_read_bytes(buf, token->uid, TOKEN_UID_LEN, TOKEN_UID_LEN) &&
          buffer_read_u8(buf, &token->symbol_len) &&
          buffer_read_bytes(buf, token->symbol, MAX_TOKEN_SYMBOL_LEN, token->symbol_len) &&
          buffer_read_u8(buf, &token->name_len) &&
          buffer_read_bytes(buf, token->name, MAX_TOKEN_NAME_LEN, token->name_len))) {
        // if any buffer read fail
        return false;
    }

    // check name and symbol for printable characters
    // This will fail if emoji or non-ascii characters are present
    if (!is_printable((char *) token->symbol, (int) token->symbol_len)) return false;
    if (!is_printable((char *) token->name, (int) token->name_len)) return false;

    return true;
}

int8_t find_token_registry_index(uint8_t *uid) {
    for (uint8_t i = 0; i < G_token_symbols.len; i++) {
        if (memcmp(uid, G_token_symbols.tokens[i].uid, TOKEN_UID_LEN) == 0) return i;
    }
    return -1;
}
