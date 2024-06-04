#include <stdint.h>
#include <stdbool.h>

#include "dispatcher.h"
#include "../constants.h"
#include "../globals.h"
#include "../types.h"
#include "../io.h"
#include "../sw.h"
#include "../common/buffer.h"
#include "../handler/get_version.h"
#include "../handler/get_address.h"
#include "../handler/get_xpub.h"
#include "../handler/sign_tx.h"
#include "../handler/sign_token_data.h"
#include "../handler/send_token_data.h"
#include "../handler/verify_token_signature.h"
#include "../handler/reset_token_signatures.h"
#include "../handler/send_create_token_data.h"

int apdu_dispatcher(const command_t *cmd) {
    if (cmd == NULL) {
        return io_send_sw(SW_INTERNAL_ERROR);
    }
    if (cmd->cla != CLA) {
        return io_send_sw(SW_CLA_NOT_SUPPORTED);
    }

    buffer_t buf = {0};

    switch (cmd->ins) {
        case GET_VERSION:
            if (cmd->p1 != 0 || cmd->p2 != 0) {
                return io_send_sw(SW_WRONG_P1P2);
            }

            return handler_get_version();
        case GET_ADDRESS:
            if (cmd->p1 != 0 || cmd->p2 != 0) {
                return io_send_sw(SW_WRONG_P1P2);
            }

            if (!cmd->data) {
                return io_send_sw(SW_WRONG_DATA_LENGTH);
            }

            buf.ptr = cmd->data;
            buf.size = cmd->lc;
            buf.offset = 0;

            return handler_get_address(&buf);
        case GET_XPUB:
            if (cmd->p1 != 0 || cmd->p2 != 0) {
                return io_send_sw(SW_WRONG_P1P2);
            }

            if (!cmd->data) {
                return io_send_sw(SW_WRONG_DATA_LENGTH);
            }

            buf.ptr = cmd->data;
            buf.size = cmd->lc;
            buf.offset = 0;

            return handler_get_xpub(&buf);
        case SIGN_TX:
            if (cmd->p1 > SIGN_TX_P1_MAX ||                                    //
                (cmd->p1 == SIGN_TX_P1_SIGN && cmd->p2 != SIGN_TX_P2_SIGN) ||  //
                (cmd->p1 == SIGN_TX_P1_DONE && cmd->p2 != SIGN_TX_P2_DONE)) {  //
                return io_send_sw(SW_WRONG_P1P2);
            }

            buf.ptr = cmd->data;
            buf.size = cmd->lc;
            buf.offset = 0;

            if ((cmd->p1 == SIGN_TX_P1_DATA && !cmd->data) ||
                (cmd->p1 == SIGN_TX_P1_SIGN && !cmd->data) ||
                (cmd->p1 == SIGN_TX_P1_DONE && cmd->data)) {
                // The data length is variable on each stage which requires data.
                // so we will just test for existence or not of data
                return io_send_sw(SW_WRONG_DATA_LENGTH);
            }

            return handler_sign_tx(&buf, (sing_tx_stage_e) cmd->p1, cmd->p2);
        case SIGN_TOKEN_DATA:
            if (cmd->p1 != 0 || cmd->p2 != 0) {
                return io_send_sw(SW_WRONG_P1P2);
            }

            buf.ptr = cmd->data;
            buf.size = cmd->lc;
            buf.offset = 0;

            return handler_sign_token_data(&buf);
        case SEND_TOKEN_DATA:
            if (cmd->p2 != 0) {
                return io_send_sw(SW_WRONG_P1P2);
            }

            buf.ptr = cmd->data;
            buf.size = cmd->lc;
            buf.offset = 0;

            return handler_send_token_data(cmd->p1 == 0, &buf);
        case VERIFY_TOKEN_SIGNATURE:
            if (cmd->p1 != 0 || cmd->p2 != 0) {
                return io_send_sw(SW_WRONG_P1P2);
            }

            buf.ptr = cmd->data;
            buf.size = cmd->lc;
            buf.offset = 0;

            return handler_verify_token_signature(&buf);
        case RESET_TOKEN_SIGNATURES:
            if (cmd->p1 != 0 || cmd->p2 != 0) {
                return io_send_sw(SW_WRONG_P1P2);
            }

            buf.ptr = cmd->data;
            buf.size = cmd->lc;
            buf.offset = 0;

            return handler_reset_token_signatures();
        case SEND_CREATE_TOKEN_DATA:
            if (cmd->p2 != 0) {
                return io_send_sw(SW_WRONG_P1P2);
            }

            buf.ptr = cmd->data;
            buf.size = cmd->lc;
            buf.offset = 0;

            return handler_send_create_token_data(cmd->p1, &buf);

        default:
            return io_send_sw(SW_INS_NOT_SUPPORTED);
    }
}
