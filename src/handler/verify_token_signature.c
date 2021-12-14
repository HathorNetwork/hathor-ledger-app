#include "verify_token_signature.h"

#include "signature.h"
#include "../globals.h"
#include "types.h"
#include "../sw.h"

int handler_verify_token_signature(buffer_t *cdata) {
    explicit_bzero(&G_context, sizeof(G_context));
    token_t token = {0};
    if (check_token_signature_from_apdu(cdata, &token))
        return io_send_sw(SW_OK);
    else
        return io_send_sw(SW_INVALID_SIGNATURE);
}
