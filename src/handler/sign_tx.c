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

/**
 * Verify that the given output address (pubkey hash) is ours and
 * may be generated from by deriving on the given bip32 path
 **/
bool verify_address(tx_output_t output, bip32_path_t path) {
    uint8_t hash[PUBKEY_HASH_LEN] = {0};
    cx_ecfp_public_key_t public_key = {0};
    cx_ecfp_private_key_t private_key = {0};
    uint8_t chain_code[32];

    derive_private_key(&private_key, chain_code, path.path, path.length);
    init_public_key(&private_key, &public_key);
    compress_public_key(public_key.W);
    hash160(public_key.W, 33, hash);

    // erase data
    explicit_bzero(&private_key, sizeof(private_key));
    explicit_bzero(&public_key, sizeof(public_key));

    // 0 means equals
    return memcmp(hash, output.pubkey_hash, PUBKEY_HASH_LEN) == 0;
}

/**
 * Read change information
 *
 * first byte:
 *  - 1 bit for the boolean `has_change_output`
 *  - the change address bip32 path length is the unsigned 7 bit integer (cast to 8 bit)
 * if `has_change_output` is true:
 *  - 1 byte for change output index
 *  - read the bip32 path (length was read on the first byte)
 *
 * The output will be on the global context for sign tx (`tx_info`)
 **/
void read_change_output_info(buffer_t *cdata) {
    uint8_t tmp;

    // 1 byte for has_change_output and bip32 path len
    if (!buffer_read_u8(cdata, &tmp)) {
        THROW(SW_WRONG_DATA_LENGTH);
    }

    // The first bit indicates the existence of a change output
    G_context.tx_info.has_change_output = (tmp & 0x80) > 0 ? true : false;

    if (G_context.tx_info.has_change_output) {
        uint8_t buffer[1 + 4*MAX_BIP32_PATH] = {0};
        buffer[0] = tmp & 0x0F;
        // 1 byte for change output index
        if (!buffer_read_u8(cdata, &G_context.tx_info.change_output_index)) {
            THROW(SW_WRONG_DATA_LENGTH);
        }

        // the remainder of the first byte was used to represent the bip32 path length of the change path
        if (buffer[0] > MAX_BIP32_PATH) {
            THROW(SW_WRONG_DATA_LENGTH);
        }

        if (cdata->size - cdata->offset < 4*buffer[0]) {
            THROW(SW_WRONG_DATA_LENGTH);
        }
        memmove(buffer+1, cdata->ptr + cdata->offset, 4 * buffer[0]);
        buffer_seek_cur(cdata, 4*buffer[0]);

        buffer_t bufdata = {
            .ptr = buffer,
            .size=1 + 4*MAX_BIP32_PATH,
            .offset = 0
        };

        // buffer holds the serialized bip32 path that was read from cdata
        if (!buffer_read_bip32_path(&bufdata, &G_context.tx_info.change_bip32_path)) {
            THROW(SW_WRONG_DATA_LENGTH);
        }
    }
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
 **/
void read_tx_data(buffer_t *cdata) {
    if (!(buffer_read_u16(cdata, &G_context.tx_info.tx_version, BE) && // read version bytes (Big Endian)
        buffer_read_u8(cdata, &G_context.tx_info.remaining_tokens) && // read number of tokens, inputs and outputs, respectively
        buffer_read_u8(cdata, &G_context.tx_info.remaining_inputs) &&
        buffer_read_u8(cdata, &G_context.tx_info.outputs_len))) {
        // if an error occurs reading
        THROW(SW_WRONG_DATA_LENGTH);
    }
}

/**
 * Add all of the input (buffer_t) to the sighash_all buffer
 * Does not move the input offset
 **/
void sighash_all_hash(buffer_t *cdata) {
    // cx_hash returns the size of the hash after adding the data, we can safely ignore it

    cx_hash(
        &G_context.tx_info.sha256.header,   // hash context pointer
        0,                                  // mode (supports: CX_LAST)
        cdata->ptr + cdata->offset,         // Input data to add to current hash
        cdata->size - cdata->offset,        // Length of input data
        NULL, 0);                           // output (if flag CX_LAST was set)
}

/**
 * Sign sighash_all with given bip32 path derived private key and send the signature
 *
 * If sighash_all is empty it means that we need to sha256d the given data to initialize it
 **/
bool sign_tx_with_key() {
    // the bip32 path should already be initialized when calling this method.

    if (G_context.bip32_path.length == 0 || G_context.bip32_path.path == (uint32_t*) NULL) {
        return io_send_sw(SW_SIGNATURE_FAIL) > 0;
    }

    cx_ecfp_private_key_t private_key = {0};
    cx_ecfp_public_key_t public_key = {0};

    uint8_t chain_code[32];

    derive_private_key(&private_key, chain_code, G_context.bip32_path.path, G_context.bip32_path.length);
    init_public_key(&private_key, &public_key);

    if (G_context.tx_info.sighash_all[0] == '\0') {
        // finish sha256 from data
        cx_hash(&G_context.tx_info.sha256.header, CX_LAST, G_context.tx_info.sighash_all, 0, G_context.tx_info.sighash_all, 32);
        // now get second sha256
        cx_sha256_init(&G_context.tx_info.sha256);
        cx_hash(&G_context.tx_info.sha256.header, CX_LAST, G_context.tx_info.sighash_all, 32, G_context.tx_info.sighash_all, 32);
    }

    uint8_t out[256] = {0};
    size_t sig_size = cx_ecdsa_sign(&private_key, CX_LAST | CX_RND_RFC6979, CX_SHA256, G_context.tx_info.sighash_all, 32, out, 256, NULL);

    explicit_bzero(&private_key, sizeof(private_key));
    explicit_bzero(&public_key, sizeof(public_key));

    // exchange signature
    // io_send_response < 0 means faillure
    return io_send_response(&(const buffer_t){.ptr = out, .size = sig_size, .offset = 0}, SW_OK) >= 0;
}

/**
 * Clean and initialize the global context for the SIGN_TX command.
 **/
void init_sign_tx_ctx() {
    explicit_bzero(&G_context, sizeof(G_context));
    G_context.req_type = CONFIRM_TRANSACTION; /// SIGN_TX

    G_context.tx_info.buffer_len = 0;
    G_context.tx_info.confirmed_outputs = 0;
    G_context.state = STATE_RECV_DATA;
    G_context.tx_info.has_change_output = false;
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
        // Safely discard data
        G_context.tx_info.remaining_tokens--;
        G_context.tx_info.buffer_len -= TOKEN_UID_LEN;
        // G_context.tx_info.elem_type = ELEM_TOKEN_UID;
        memmove(G_context.tx_info.buffer, G_context.tx_info.buffer + TOKEN_UID_LEN, G_context.tx_info.buffer_len);
    } else if(G_context.tx_info.remaining_inputs > 0) {
        // Since we are using this input serialization to sign the transaction, we must have data_len == 0
        // So we have 32 bytes of TX id + 1 byte of index + 2 bytes of data length = 35 bytes
        // After confirming that the data_len is 0, we can safely ignore.
        // can read input? if not, request more data
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
        // G_context.tx_info.elem_type = ELEM_INPUT;
        memmove(G_context.tx_info.buffer, G_context.tx_info.buffer + TX_INPUT_LEN, G_context.tx_info.buffer_len);
    } else if (G_context.tx_info.current_output < G_context.tx_info.outputs_len) {
        // Here, we have already read all tokens and inputs, now we will read the output bytes
        // User confirmation of the information here is required, so we shall parse the data
        // and save it for the UI functions to read later.
        tx_output_t output = {0};

        // read output (function is responsible to THROW if more data is required to parse the output)
        size_t output_len = parse_output(
            G_context.tx_info.buffer,
            G_context.tx_info.buffer_len,
            &output);

        // set output index and prepare for next parse
        output.index = G_context.tx_info.current_output++;
        // G_context.tx_info.elem_type = ELEM_OUTPUT;

        // If this output was a change output, we require extra validation
        if (G_context.tx_info.has_change_output && G_context.tx_info.change_output_index == output.index) {
            if (!verify_address(output, G_context.tx_info.change_bip32_path)) {
                THROW(TX_STATE_ERR);
            }
        }
        // We have already read and parsed the output, move the buffer so we can parse the next one
        G_context.tx_info.buffer_len -= output_len;
        memmove(G_context.tx_info.buffer, G_context.tx_info.buffer + output_len, G_context.tx_info.buffer_len);
        // Save the output on the global context for user confirmation
        output_len = sizeof(G_context.tx_info.outputs) / sizeof(G_context.tx_info.outputs[0]);
        if (G_context.tx_info.buffer_output_index >= output_len) {
            // prevent overflow
            THROW(TX_STATE_ERR);
        }
        G_context.tx_info.outputs[G_context.tx_info.buffer_output_index++] =  output;
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
            // for (;;) _decode_elements();
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
        // read change output and sighash
        read_change_output_info(cdata);
        // sighash_all_hash won't move cdata.offset
        sighash_all_hash(cdata);
        read_tx_data(cdata);

        if(!buffer_copy(cdata, G_context.tx_info.buffer, 300 - G_context.tx_info.buffer_len)) {
            THROW(SW_WRONG_DATA_LENGTH);
        }

        G_context.tx_info.buffer_len += cdata->size - cdata->offset;
    } else {
        // copy sighash_all data to sha256 context
        // any call on the data stage after the first must be part of the sighash_all
        // add it to the buffer and parse the elements
        sighash_all_hash(cdata);
        // move the same data to decode buffer
        if(!buffer_copy(cdata, G_context.tx_info.buffer + G_context.tx_info.buffer_len, 300 - G_context.tx_info.buffer_len)) {
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
            if (G_context.tx_info.buffer_output_index != 0) {
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
    G_context.tx_info.buffer_output_index = 0;
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
            if(!sign_tx_with_key()) return -1;
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
            if(!receive_data(cdata, chunk)) return -1;
            break;

        default:
            explicit_bzero(&G_context, sizeof(G_context));
            ui_menu_main();
            return io_send_sw(SW_BAD_STATE);
    }

    return 0;
}
