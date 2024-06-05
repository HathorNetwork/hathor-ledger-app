#include "send_create_token_data.h"

#include "token_parser.h"
#include "signature.h"
#include "../globals.h"
#include "types.h"
#include "../sw.h"
#include "../common/buffer.h"
#include "../common/format.h"
#include "../ui/display.h"

int handler_send_create_token_data(uint8_t token_type, buffer_t *cdata) {
    explicit_bzero(&G_context, sizeof(G_context));
    explicit_bzero(&G_token_symbols, sizeof(G_token_symbols));

    // version, symbol, name (and +data if is_nft)
    if (!(buffer_read_u8(cdata, &G_context.token.version) &&
          G_context.token.version == 1 &&
          buffer_read_u8(cdata, &G_context.token.name_len) &&
          buffer_read_bytes(cdata,
                            G_context.token.name,
                            MAX_TOKEN_NAME_LEN,
                            G_context.token.name_len) &&
          buffer_read_u8(cdata, &G_context.token.symbol_len) &&
          buffer_read_bytes(cdata,
                            G_context.token.symbol,
                            MAX_TOKEN_SYMBOL_LEN,
                            G_context.token.symbol_len))) {
        // if any buffer read fail due to insufficient data
        return io_send_sw(SW_WRONG_DATA_LENGTH);
    }

    // check name and symbol for printable characters
    // This will fail if emoji or non-ascii characters are present
    if (!is_printable((char *) G_context.token.symbol, (int) G_context.token.symbol_len))
        return io_send_sw(SW_UNPRINTABLE_STRING);
    if (!is_printable((char *) G_context.token.name, (int) G_context.token.name_len))
        return io_send_sw(SW_UNPRINTABLE_STRING);

    // NFT token type is 0X01
    if (token_type == 1) {
        // read data
        if (!(buffer_read_u8(cdata, &G_context.nft_data_len) &&
              buffer_read_bytes(cdata, G_context.nft_data, 150, G_context.nft_data_len))) {
            // if any buffer read fail due to insufficient data
            return io_send_sw(SW_WRONG_DATA_LENGTH);
        }
    }

    // Start flow to ask user for confirmation
    PRINTF("[+] [send_create_token_data] Token version=%d, namelen=%d, symbollen=%d\n",
           G_context.token.version,
           G_context.token.name_len,
           G_context.token.symbol_len);

    return ui_display_create_token_data();
}
