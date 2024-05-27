#include <stdint.h>   // uint*_t
#include <stdbool.h>  // bool
#include <stddef.h>   // size_t
#include <string.h>   // explicit_bzero, memmove

#include "os.h"
#include "cx.h"

#include "sign_tx.h"

#include "../hathor.h"
#include "../sw.h"
#include "../globals.h"
#include "../ui/display.h"
#include "../ui/menu.h"
#include "../common/buffer.h"
#include "../common/bip32.h"
#include "../common/read.h"
#include "../transaction/types.h"
#include "../transaction/deserialize.h"
#include "../token/token_parser.h"
#include "../token/types.h"

/**
 * Verify that the given output address (pubkey hash) is ours and
 * may be generated from by deriving on the given bip32 path
 **/
bool verify_change(change_output_info_t *info, tx_output_t output) {
    uint8_t hash[PUBKEY_HASH_LEN] = {0};
    cx_ecfp_public_key_t public_key = {0};
    cx_ecfp_private_key_t private_key = {0};
    uint8_t chain_code[32];
    int error = 0;

    if (info == NULL) {
        return false;
    }

    CX_CHECK(derive_private_key(&private_key, chain_code, info->path.path, info->path.length));
    CX_CHECK(init_public_key(&private_key, &public_key));
    CX_CHECK(compress_public_key(public_key.W, sizeof(public_key.W) / sizeof(public_key.W[0])));
    CX_CHECK(hash160(public_key.W, 33, hash, PUBKEY_HASH_LEN));

end:
    // erase data
    explicit_bzero(&private_key, sizeof(private_key));
    explicit_bzero(&public_key, sizeof(public_key));

    if (error) {
        THROW(SW_INTERNAL_ERROR);
    }

    // 0 means equals
    return memcmp(hash, output.pubkey_hash, PUBKEY_HASH_LEN) == 0;
}

/**
 * Read change information
 *
 * first byte:
 *  - 1 bit for the number of change outputs
 *  - for each change output:
 *      - 1 byte for change output index
 *      - read the bip32 path
 *
 * The outputs will be on the global context for sign tx (`tx_info`)
 *
 * returns a 16 bit error code if it fails and 0 on success
 **/
uint16_t read_change_info_v1(buffer_t *cdata) {
    if (!buffer_read_u8(cdata, &G_context.tx_info.change_len)) {
        return SW_WRONG_DATA_LENGTH;
    }

    if (G_context.tx_info.change_len > TX_MAX_TOKENS + 1) {
        // Some token change is duplicated or we have more tokens than allowed
        return SW_WRONG_DATA_LENGTH;
    }

    // TX_MAX_TOKENS + 1 because we allow 1 change for each token + HTR
    uint8_t known_indexes[TX_MAX_TOKENS + 1] = {0};
    uint8_t indexes_len = 0;

    for (uint8_t i = 0; i < G_context.tx_info.change_len; i++) {
        change_output_info_t *info = &G_context.tx_info.change_info[i];

        // 1 byte for change output index
        if (!buffer_read_u8(cdata, &info->index)) {
            return SW_WRONG_DATA_LENGTH;
        }

        // check for duplicates
        for (uint8_t j = 0; j < indexes_len; j++) {
            if (info->index == known_indexes[j]) {
                // This output index was already read
                // This can happen if the change list is malformed.
                PRINTF("[-] [sign_tx] Duplicate change output index %d\n", info->index);
                return SW_INVALID_TX;
            }
        }
        known_indexes[indexes_len++] = info->index;

        // bip32 path (path length + path data)
        if (!buffer_read_bip32_path(cdata, &info->path)) {
            return SW_WRONG_DATA_LENGTH;
        }
    }

    return 0;
}

/**
 * Read change information
 *
 * first byte:
 *  - 1 bit for the number of change outputs
 *  - for each change output:
 *      - 1 byte for change output index
 *      - read the bip32 path
 *
 * The outputs will be on the global context for sign tx (`tx_info`)
 *
 * returns a 16 bit error code if it fails and 0 on success
 **/
uint16_t read_change_info_old_protocol(uint8_t change_byte, buffer_t *cdata) {
    uint8_t buffer[1 + 4 * MAX_BIP32_PATH] = {0};
    // Old protocol only allow 1 change output
    // and we already checked that there is an output present
    G_context.tx_info.change_len = 1;
    change_output_info_t *info = &G_context.tx_info.change_info[0];

    if (!buffer_read_u8(cdata, &info->index)) {
        return SW_WRONG_DATA_LENGTH;
    }

    buffer[0] = change_byte & 0x0F;

    // validate that the path length is valid
    if (buffer[0] > MAX_BIP32_PATH) {
        return SW_WRONG_DATA_LENGTH;
    }
    // check we have enough data to read the path
    if (cdata->size - cdata->offset < 4 * buffer[0]) {
        return SW_WRONG_DATA_LENGTH;
    }

    // buffer+1 has 4*MAX_BIP32_PATH capacity
    // buffer[0] is limited to MAX_BIP32_PATH
    memmove(buffer + 1, cdata->ptr + cdata->offset, 4 * buffer[0]);
    buffer_seek_cur(cdata, 4 * buffer[0]);
    buffer_t bufdata = {.ptr = buffer, .size = 1 + 4 * MAX_BIP32_PATH, .offset = 0};

    // bip32 path (path length + path data)
    if (!buffer_read_bip32_path(&bufdata, &info->path)) {
        return SW_WRONG_DATA_LENGTH;
    }

    return 0;
}

/**
 * Read change information
 *
 * First byte is the version byte for the protocol (for transactions)
 * It will be used to differentiate the old and new protocols
 *
 * returns a 16 bit error code if it fails and 0 on success
 **/
uint16_t read_change_info(buffer_t *cdata) {
    uint8_t proto_version;
    if (!buffer_read_u8(cdata, &proto_version)) {
        return SW_WRONG_DATA_LENGTH;
    }

    // check first byte for backwards compatibility
    // - if proto_version == 0: old protocol, no change
    // - else:
    //      - first bit == 1: old protocol, with change
    //      - else:
    //          - proto_version == 1: new protocol
    //          - else: error

    if (proto_version == 0) {
        // old protocol, no change
        G_context.tx_info.change_len = 0;
        return 0;
    }

    // Old protocol with change
    if (proto_version & 0x80) {
        return read_change_info_old_protocol(proto_version, cdata);
    }

    // Protocol v1
    if (proto_version == 1) {
        return read_change_info_v1(cdata);
    }

    // error, unknown change protocol
    PRINTF("[-] [sign_tx] Unknown change protocol version %d\n", proto_version);
    return SW_INVALID_TX;
}

/**
 * Read the info needed to parse the sighash_all data
 *
 * 2 bytes for tx version
 * - 1 : HTR Transaction (Only supported version, currently)
 *
 * 1 byte for number of tokens
 * 1 byte for number of inputs
 * 1 byte for number of outputs
 *
 * Obs: This is only on the first call with chunk=0, so it should never fail for lack of data
 *
 * The output will be on the global context for sign tx (`tx_info`)
 *
 * returns a 16 bit error code if it fails and 0 on success
 **/
uint16_t read_tx_data(buffer_t *cdata) {
    if (!(buffer_read_u16(cdata,
                          &G_context.tx_info.tx_version,
                          BE) &&  // read version bytes (Big Endian)
          buffer_read_u8(cdata,
                         &G_context.tx_info.remaining_tokens) &&  // read number of tokens, inputs
                                                                  // and outputs, respectively
          buffer_read_u8(cdata, &G_context.tx_info.remaining_inputs) &&
          buffer_read_u8(cdata, &G_context.tx_info.outputs_len))) {
        // if an error occurs reading
        return SW_WRONG_DATA_LENGTH;
    }

    for (uint8_t i = 0; i < G_context.tx_info.change_len; i++) {
        // check that all change indexes are inside the outputs array
        if (G_context.tx_info.change_info[i].index >= G_context.tx_info.outputs_len) {
            PRINTF("[-] [sign_tx] Change output index %d cannot be an output since length is %d\n",
                   G_context.tx_info.change_info[i].index,
                   G_context.tx_info.outputs_len);
            return SW_INVALID_TX;
        }
    }
    if (G_context.tx_info.change_len > G_context.tx_info.outputs_len) {
        // We have more change indexes than outputs, meaning the change list is malformed
        // This is unreachable since we do not allow duplicates and no index is higher than the
        // number of outputs even so we should check for this.
        PRINTF("[-] [sign_tx] More change indexes than outputs\n");
        return SW_INVALID_TX;
    }

    return 0;
}

/**
 * Add all of the input (buffer_t) to the sighash_all buffer
 * Does not move the input offset
 **/
void sighash_all_hash(buffer_t *cdata) {
    // cx_hash returns the size of the hash after adding the data, we can safely ignore it

    CX_THROW(cx_hash_no_throw(&G_context.tx_info.sha256.header,  // hash context pointer
                              0,                                 // mode (supports: CX_LAST)
                              cdata->ptr + cdata->offset,   // Input data to add to current hash
                              cdata->size - cdata->offset,  // Length of input data
                              NULL,
                              0));  // output (if flag CX_LAST was set)
}

/**
 * Sign sighash_all with given bip32 path derived private key and send the signature
 *
 * If sighash_all is empty it means that we need to sha256d the given data to initialize it
 **/
bool sign_tx_with_key() {
    // the bip32 path should already be initialized when calling this method.

    if (G_context.bip32_path.length == 0 || G_context.bip32_path.path == (uint32_t *) NULL) {
        return io_send_sw(SW_SIGNATURE_FAIL) > 0;
    }

    cx_ecfp_private_key_t private_key = {0};

    uint8_t chain_code[32];
    cx_err_t error = CX_OK;

    CX_CHECK(derive_private_key(&private_key,
                                chain_code,
                                G_context.bip32_path.path,
                                G_context.bip32_path.length));

    if (G_context.tx_info.sighash_all[0] == '\0') {
        // finish sha256 from data
        CX_CHECK(cx_hash_no_throw(&G_context.tx_info.sha256.header,
                                  CX_LAST,
                                  G_context.tx_info.sighash_all,
                                  0,
                                  G_context.tx_info.sighash_all,
                                  32));
        // now get second sha256
        cx_sha256_init(&G_context.tx_info.sha256);
        CX_CHECK(cx_hash_no_throw(&G_context.tx_info.sha256.header,
                                  CX_LAST,
                                  G_context.tx_info.sighash_all,
                                  32,
                                  G_context.tx_info.sighash_all,
                                  32));
    }

    uint8_t out[256] = {0};
    size_t sig_size = 256;
    CX_CHECK(cx_ecdsa_sign_no_throw(&private_key,
                                    CX_LAST | CX_RND_RFC6979,
                                    CX_SHA256,
                                    G_context.tx_info.sighash_all,
                                    32,
                                    out,
                                    &sig_size,
                                    NULL));

end:
    explicit_bzero(&private_key, sizeof(private_key));

    if (error) {
        return io_send_sw(SW_INTERNAL_ERROR) > 0;
    }

    // exchange signature
    // io_send_response < 0 means faillure
    return io_send_response(&(const buffer_t){.ptr = out, .size = sig_size, .offset = 0}, SW_OK) >=
           0;
}

/**
 * Clean and initialize the global context for the SIGN_TX command.
 **/
void init_sign_tx_ctx() {
    explicit_bzero(&G_context, sizeof(G_context));
    G_context.req_type = CONFIRM_TRANSACTION;  /// SIGN_TX

    G_context.tx_info.buffer_len = 0;
    G_context.tx_info.confirmed_outputs = 0;
    G_context.state = STATE_RECV_DATA;
    G_context.tx_info.change_len = 0;
    G_context.tx_info.current_output = 0;

    cx_sha256_init(&G_context.tx_info.sha256);
    G_context.tx_info.sighash_all[0] = '\0';
}

/**
 * Decode the elements from the sighash_all data
 *
 * - tokens
 *   - currently no validation is done here
 * - inputs
 *   - currently no validation is done here
 * - outputs
 *   - parse and validate
 *   - if it's the change, verify the pubkey hash of the output
 *   - once all the buffer for this call is parsed
 *      throw TX_STATE_READY: will start the ux flow to confirm the outputs
 *
 * @return true if there is more data left on the buffer to decode, false otherwise.
 **/
bool _decode_elements() {
    // test if there are more tokens, inputs and outputs in this order
    // if there are, try to read from buffer
    // if any are incomplete, inform caller to send more data
    if (G_context.tx_info.remaining_tokens > 0) {
        // Here we read the token UIDs, no validation or confirmation to be done.
        // After reading 32 bytes of data, we can safely ignore the token.
        // Can read token? if not, request more data.
        if (G_context.tx_info.buffer_len < TOKEN_UID_LEN) {
            // still decoding tokens, but this one was divided between calls
            THROW(TX_STATE_READY);
        }
        uint8_t uid[TOKEN_UID_LEN] = {0};
        memmove(uid, G_context.tx_info.buffer, TOKEN_UID_LEN);

        // check uid is on registry
        int8_t registry_index = find_token_registry_index(uid);
        if (registry_index == -1) THROW(TX_STATE_ERR);  // not on registry, token MUST be verified
        G_context.tx_info.tokens[G_context.tx_info.tokens_len++] =
            &G_token_symbols.tokens[registry_index];

        G_context.tx_info.remaining_tokens--;
        G_context.tx_info.buffer_len -= TOKEN_UID_LEN;
        memmove(G_context.tx_info.buffer,
                G_context.tx_info.buffer + TOKEN_UID_LEN,
                G_context.tx_info.buffer_len);
    } else if (G_context.tx_info.remaining_inputs > 0) {
        // Since we are using this input serialization to sign the transaction, we must have
        // data_len == 0 So we have 32 bytes of TX id + 1 byte of index + 2 bytes of data length =
        // 35 bytes After confirming that the data_len is 0, we can safely ignore. can read input?
        // if not, request more data
        if (G_context.tx_info.buffer_len < TX_INPUT_LEN) {
            THROW(TX_STATE_READY);
        }
        // Check data_len
        uint16_t input_data_len = read_u16_be(G_context.tx_info.buffer, 33);
        if (input_data_len > 0) {
            THROW(TX_STATE_ERR);
        }

        // Safely discard data
        G_context.tx_info.remaining_inputs--;
        G_context.tx_info.buffer_len -= TX_INPUT_LEN;
        memmove(G_context.tx_info.buffer,
                G_context.tx_info.buffer + TX_INPUT_LEN,
                G_context.tx_info.buffer_len);
    } else if (G_context.tx_info.current_output < G_context.tx_info.outputs_len) {
        // Here, we have already read all tokens and inputs, now we will read the output bytes
        // User confirmation of the information here is required, so we shall parse the data
        // and save it for the UI functions to read later.
        tx_output_t output = {0};

        // read output (function is responsible to THROW if more data is required to parse the
        // output)
        size_t output_len =
            parse_output(G_context.tx_info.buffer, G_context.tx_info.buffer_len, &output);

        // check the output token_data is correct
        if (output.token_data & TOKEN_DATA_AUTHORITY_MASK) {
            // authority outputs are not allowed!
            THROW(TX_STATE_ERR);
        }
        // We exclude the equal case since index == 0 means HTR
        if ((output.token_data & TOKEN_DATA_INDEX_MASK) > G_context.tx_info.tokens_len) {
            // index out of bounds
            THROW(TX_STATE_ERR);
        }

        // token exists and it's in the tokens array

        // set output index and prepare for next parse
        output.index = G_context.tx_info.current_output++;

        // If this output is a change output, we require extra validation
        if (G_context.tx_info.change_len > 0) {
            // search for an output with this index
            for (uint8_t i = 0; i < G_context.tx_info.change_len; i++) {
                change_output_info_t *info = &G_context.tx_info.change_info[i];
                // check if index matches
                if (output.index == info->index) {
                    // verify change output is going to user's wallet
                    if (!verify_change(info, output)) {
                        THROW(TX_STATE_ERR);
                    }
                }
            }
        }
        // We have already read and parsed the output, move the buffer so we can parse the next one
        G_context.tx_info.buffer_len -= output_len;
        memmove(G_context.tx_info.buffer,
                G_context.tx_info.buffer + output_len,
                G_context.tx_info.buffer_len);
        // Save the output on the global context for user confirmation
        output_len = sizeof(G_context.tx_info.outputs) / sizeof(G_context.tx_info.outputs[0]);
        if (G_context.tx_info.buffer_output_len >= output_len) {
            // prevent overflow
            THROW(TX_STATE_ERR);
        }
        G_context.tx_info.outputs[G_context.tx_info.buffer_output_len++] = output;
    } else {
        // We've reached the end of what we should read but the buffer isn't empty
        THROW(TX_STATE_ERR);
    }

    // Is there more data to parse?
    return G_context.tx_info.buffer_len != 0;
}

/**
 * A try/catch context for parsing the received data
 *
 * The actual decode element function will be called
 * indefinetely until something is thrown, an element
 * should be parsed on each call
 *
 * Obs: any error thrown in this context will be ignored
 *      except for SW_TX_PARSING_FAIL and the expected tx_decoder_state_e
 *      in case of a SW_TX_PARSING_FAIL, TX_STATE_ERR will be returned
 *
 * @return state of parsed buffer
 **/
tx_decoder_state_e decode_elements() {
    volatile tx_decoder_state_e result;
    BEGIN_TRY {
        TRY {
            while (_decode_elements()) {
                ;
            }
            // No more data to decode on this buffer.
            result = TX_STATE_READY;
        }
        CATCH(SW_TX_PARSING_FAIL) {
            result = TX_STATE_ERR;
        }
        CATCH_OTHER(e) {
            result = e;
        }
        FINALLY {
        }
    }
    END_TRY;
    return result;
}

/**
 * Handler for SIGN_TX first stage
 *
 * Initialize context if first call, read and parse the data
 * Should start the UX_FLOW if the user confirmation is required
 *
 **/
bool receive_data(buffer_t *cdata, uint8_t chunk) {
    // The transaction will be divided into chunks of data
    // each chunk will be numbered, chunk==0 means this is the start of the data.
    if (chunk == 0) {
        if (G_context.state == STATE_RECV_DATA) {
            // sent first chunk twice? return error
            explicit_bzero(&G_context, sizeof(G_context));
            THROW(SW_BAD_STATE);
        }

        init_sign_tx_ctx();
        uint16_t error_code = 0;
        // read change output and sighash
        error_code = read_change_info(cdata);
        if (error_code != 0) {
            explicit_bzero(&G_context, sizeof(G_context));
            THROW(error_code);
        }
        // sighash_all_hash won't move cdata.offset
        sighash_all_hash(cdata);
        error_code = read_tx_data(cdata);
        if (error_code != 0) {
            explicit_bzero(&G_context, sizeof(G_context));
            THROW(error_code);
        }

        if (!buffer_copy(cdata, G_context.tx_info.buffer, 300 - G_context.tx_info.buffer_len)) {
            explicit_bzero(&G_context, sizeof(G_context));
            THROW(SW_WRONG_DATA_LENGTH);
        }

        G_context.tx_info.buffer_len += cdata->size - cdata->offset;
    } else {
        // copy sighash_all data to sha256 context
        // any call on the data stage after the first must be part of the sighash_all
        // add it to the buffer and parse the elements
        sighash_all_hash(cdata);
        // move the same data to decode buffer
        if (!buffer_copy(cdata,
                         G_context.tx_info.buffer + G_context.tx_info.buffer_len,
                         300 - G_context.tx_info.buffer_len)) {
            explicit_bzero(&G_context, sizeof(G_context));
            THROW(SW_WRONG_DATA_LENGTH);
        }
        G_context.tx_info.buffer_len += cdata->size - cdata->offset;
    }

    switch (decode_elements()) {
        case TX_STATE_ERR:
            explicit_bzero(&G_context, sizeof(G_context));
            io_send_sw(SW_INVALID_TX);
            ui_menu_main();
            return true;
        case TX_STATE_READY:
            if (G_context.tx_info.buffer_output_len != 0) {
                // We have an output for the user to confirm.
                // Validate parsed outputs and return sw_ok upon user confirmation
                // if the last output was reached, confirm the transaction send.
                return ui_display_tx_outputs() == 0;
            } else {
                // We don't have an output to confirm
                // Occurs when there's too many tokens/inputs
                // return SW_OK to request more data
                io_send_sw(SW_OK);
                return true;
            }
    }
    return false;
}

int handler_sign_tx(buffer_t *cdata, sing_tx_stage_e stage, uint8_t chunk) {
    // switch(stage) actions for each stage
    // - SIGN_TX_STAGE_DONE: signals the end of the call
    // - SIGN_TX_STAGE_SIGN: TX was received and approved by user, caller requests signing
    // - SIGN_TX_STAGE_DATA: Receiving TX in chunks
    G_context.tx_info.display_index = 0;
    G_context.tx_info.buffer_output_len = 0;
    switch (stage) {
        case SIGN_TX_STAGE_DONE:
            // Caller is done with this request, cleanup and return SW_OK.
            explicit_bzero(&G_context, sizeof(G_context));
            G_context.state = STATE_NONE;
            ui_menu_main();
            return io_send_sw(SW_OK);

        case SIGN_TX_STAGE_SIGN:
            // Caller passes the bip32 path of the key needed to sign the request
            if (G_context.state != STATE_APPROVED) {
                // The user must have approved already
                return io_send_sw(SW_BAD_STATE);
            }

            if (!buffer_read_bip32_path(cdata, &G_context.bip32_path)) {
                return io_send_sw(SW_WRONG_DATA_LENGTH);
            }

            // sign with input key and return SW_OK
            if (!sign_tx_with_key()) return -1;
            break;

        case SIGN_TX_STAGE_DATA:
            // Caller will pass the transaction and metadata needed to approve and sign.
            if (G_context.state == STATE_APPROVED) {
                // cannot receive more data after user approval
                explicit_bzero(&G_context, sizeof(G_context));
                ui_menu_main();
                return io_send_sw(SW_BAD_STATE);
            }
            if (chunk > 0 && G_context.state != STATE_RECV_DATA) {
                // Some stage before this failed but caller sent data anyway
                // reject with SW_BAD_STATE
                explicit_bzero(&G_context, sizeof(G_context));
                ui_menu_main();
                return io_send_sw(SW_BAD_STATE);
            }
            if (!receive_data(cdata, chunk)) return -1;
            break;

        default:
            explicit_bzero(&G_context, sizeof(G_context));
            ui_menu_main();
            return io_send_sw(SW_BAD_STATE);
    }

    return 0;
}
