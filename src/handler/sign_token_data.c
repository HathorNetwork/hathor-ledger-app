#include "sign_token_data.h"

#include "token_parser.h"
#include "../globals.h"
#include "../sw.h"
#include "../ui/display.h"

int handler_sign_token_data(buffer_t *cdata) {
    explicit_bzero(&G_context, sizeof(G_context));
    // parse info on token to global context
    if (!parse_token(cdata, &G_context.token)) return io_send_sw(SW_INVALID_SIGNATURE);
    return ui_display_sign_token_data();
}
