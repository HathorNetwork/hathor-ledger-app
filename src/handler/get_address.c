#include <stdint.h>  // uint*_t

#include "get_address.h"
#include "../globals.h"
#include "../io.h"
#include "../sw.h"
#include "../types.h"
#include "../ui/display.h"
#include "common/buffer.h"
#include "../hathor.h"

void derive_address() {
    cx_ecfp_private_key_t private_key = {0};
    cx_ecfp_public_key_t public_key = {0};
    uint8_t chain_code[32];

    // derive for bip32 path
    derive_private_key(&private_key, chain_code, G_context.bip32_path.path, G_context.bip32_path.length);
    init_public_key(&private_key, &public_key);

    memmove(G_context.pk_info.raw_public_key, public_key.W, public_key.W_len);

    explicit_bzero(&private_key, sizeof(private_key));
    explicit_bzero(&public_key, sizeof(public_key));
}

int handler_get_address(buffer_t *cdata) {
    explicit_bzero(&G_context, sizeof(G_context));
    G_context.req_type = CONFIRM_ADDRESS;
    G_context.state = STATE_NONE;

    if (!buffer_read_bip32_path(cdata, &G_context.bip32_path)) {
        return io_send_sw(SW_WRONG_DATA_LENGTH);
    }

    derive_address();

    return ui_display_confirm_address();
}
