#include <string.h>

#include "parser.h"

#include "../common/buffer.h"


bool parse_token(buffer_t *buf, token_t *token) {
    // read uid, len(symbol), symbol, len(name), name, version
    if (!(buffer_read_bytes(buf, token->uid, TOKEN_UID_LEN, TOKEN_UID_LEN) &&
          buffer_read_u8(buf, &token->symbol_len) &&
          buffer_read_bytes(buf, token->symbol, TOKEN_SYMBOL_LEN, token->symbol_len) &&
          buffer_read_u8(buf, &token->name_len) &&
          buffer_read_bytes(buf, token->name, TOKEN_NAME_LEN, token->name_len) &&
          buffer_read_u8(buf, &token->version))) {
        // if any buffer read fail
        return false;
    }

    // check name and symbol for printable characters
    // This will fail if emoji or non-ascii characters are present
    for(int i=0; i<token->symbol_len; i++) {
        if ((uint8_t)token->symbol[i] < 0x80u) continue;
        return false;
    }

    for(int i=0; i<token->name_len; i++) {
        if ((uint8_t)token->name[i] < 0x80u) continue;
        return false;
    }

    return true;
}
