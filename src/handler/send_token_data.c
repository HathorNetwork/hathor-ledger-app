#include "send_token_data.h"

#include "token_parser.h"
#include "signature.h"
#include "../globals.h"
#include "types.h"
#include "../sw.h"

int handler_send_token_data(bool first, buffer_t *cdata) {
    token_t token = {0};

    if (first) {
        explicit_bzero(&G_context, sizeof(G_context));
        explicit_bzero(&G_token_symbols, sizeof(G_token_symbols));
    }

    if (G_token_symbols.len >= TX_MAX_TOKENS) {
        // too many tokens
        explicit_bzero(&G_token_symbols, sizeof(G_token_symbols));
        return io_send_sw(SW_WRONG_DATA_LENGTH);
    }

    if (!check_token_signature_from_apdu(cdata, &token)) return io_send_sw(SW_INVALID_SIGNATURE);

    // save token/symbol on global ctx
    token_symbol_t *token_symbol = &G_token_symbols.tokens[G_token_symbols.len++];

    memmove(&token_symbol->uid, &token.uid, sizeof(token_uid_t));
    memmove(&token_symbol->symbol, &token.symbol, token.symbol_len);
    token_symbol->symbol[token.symbol_len] = '\0';

    return io_send_sw(SW_OK);
}
