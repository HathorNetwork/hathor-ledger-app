#include "verify_token_signature.h"

#include "signature.h"
#include "../globals.h"
#include "types.h"
#include "../sw.h"

int handler_verify_token_signature(buffer_t *cdata) {
    explicit_bzero(&G_context, sizeof(G_context));
    token_t token = {0};
    int is_token_verified = check_token_signature_from_apdu(cdata, &token);
    switch (is_token_verified) {
        case 0:
            return io_send_sw(SW_OK);
        case 1:
            return io_send_sw(SW_INVALID_SIGNATURE);
        default:
            PRINTF("[-] [verify_token_signature] Internal error %d\n", is_token_verified);
            return io_send_sw(SW_INTERNAL_ERROR);
    }
}
