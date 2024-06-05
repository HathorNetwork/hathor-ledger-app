#include <stdbool.h>  // bool

#include "validate.h"
#include "../menu.h"
#include "../../sw.h"
#include "../../io.h"
#include "../../globals.h"
#include "../../helper/send_response.h"

void ui_action_confirm_xpub(bool choice) {
    if (choice) {
        helper_send_response_xpub();
    } else {
        explicit_bzero(&G_context, sizeof(G_context));
        io_send_sw(SW_DENY);
    }

    ui_menu_main();
}

void ui_action_confirm_address(bool choice) {
    if (choice) {
        io_send_sw(SW_OK);
    } else {
        explicit_bzero(&G_context, sizeof(G_context));
        io_send_sw(SW_DENY);
    }

    ui_menu_main();
}

void ui_action_sign_token_data(bool choice) {
    if (choice) {
        helper_send_token_data_signature();
    } else {
        explicit_bzero(&G_context, sizeof(G_context));
        io_send_sw(SW_DENY);
    }

    ui_menu_main();
}

void ui_action_send_create_token(bool choice) {
    if (choice) {
        token_symbol_t *token_symbol = &G_token_symbols.tokens[G_token_symbols.len++];
        memmove(&token_symbol->symbol, &G_context.token.symbol, G_context.token.symbol_len);
        token_symbol->symbol[G_context.token.symbol_len] = '\0';
        io_send_sw(SW_OK);
    } else {
        explicit_bzero(&G_context, sizeof(G_context));
        explicit_bzero(&G_token_symbols, sizeof(G_token_symbols));
        io_send_sw(SW_DENY);
    }

    ui_menu_main();
}
