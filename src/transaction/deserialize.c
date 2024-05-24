#include <stdbool.h>  // bool
#include <string.h>   // memset, explicit_bzero, memmove

#include "deserialize.h"

#include "os.h"  // THROW

#include "../sw.h"
#include "constants.h"
#include "../common/buffer.h"
#include "types.h"

bool is_authority_output(uint8_t token_data) {
    return (token_data & TOKEN_DATA_AUTHORITY_MASK) > 0;
}

bool is_mint_authority(uint8_t token_data, uint64_t value) {
    return is_authority_output(token_data) && value == MINT_AUTHORITY_MASK;
}

bool is_melt_authority(uint8_t token_data, uint64_t value) {
    return is_authority_output(token_data) && value == MELT_AUTHORITY_MASK;
}

/**
 * XXX: considering only P2PKH, without timelock
 * Validates that a script has the format of P2PKH. Throws an exception if doesn't.
 * P2PKH scripts have the format:
 *   [OP_DUP(1), OP_HASH160(1), pubkey_hash_len(1), pubkey_hash(20), OP_EQUALVERIFY(1),
 * OP_CHECKSIG(1)]
 */
void validate_p2pkh_script(buffer_t *in, size_t script_len) {
    uint8_t p2pkh[] = {OP_DUP, OP_HASH160, PUBKEY_HASH_LEN, OP_EQUALVERIFY, OP_CHECKSIG};

    if (in == NULL) {
        THROW(SW_INTERNAL_ERROR);
    }

    if (script_len != 25) {
        THROW(SW_TX_PARSING_FAIL);
    }

    if (in->size - in->offset < 25) {
        THROW(TX_STATE_READY);
    }

    if (memcmp(p2pkh, in->ptr + in->offset, 3) != 0 ||
        memcmp(p2pkh + 3, in->ptr + in->offset + PUBKEY_HASH_LEN + 3, 2) != 0) {
        THROW(SW_TX_PARSING_FAIL);
    }
}

void parse_output_value(buffer_t *buf, uint64_t *value) {
    if (buf == NULL) {
        THROW(SW_INTERNAL_ERROR);
    }
    // if first bit is 1 value has length 8 bytes, otherwise it's 4 bytes
    bool flag = (bool) (0x80u & buf->ptr[0]);
    if (flag) {
        uint64_t tmp = 0;
        if (!buffer_read_u64(buf, &tmp, BE)) {
            THROW(TX_STATE_READY);
        }
        // To use the first bit to indicate length of 8 bytes
        // we serialized the negative value of the 8 byte int so we need to correct it
        tmp = (-1) * tmp;
        *value = tmp;
    } else {
        uint32_t tmp = 0;
        if (!buffer_read_u32(buf, &tmp, BE)) {
            THROW(TX_STATE_READY);
        }
        // we don't need to correct anything
        *value = (uint64_t) tmp;
    }
}

size_t parse_output(uint8_t *in, size_t inlen, tx_output_t *output) {
    uint16_t script_len;
    buffer_t buf = {.ptr = in, .size = inlen, .offset = 0};

    if (in == NULL || output == NULL) {
        THROW(SW_INTERNAL_ERROR);
    }
    parse_output_value(&buf, &output->value);

    // read token data and script length
    if (!(buffer_read_u8(&buf, &output->token_data) && buffer_read_u16(&buf, &script_len, BE))) {
        THROW(TX_STATE_READY);
    }
    // validate script and extract pubkey hash
    validate_p2pkh_script(&buf, script_len);
    // validate already asserted the length for this extraction
    memmove(output->pubkey_hash, buf.ptr + buf.offset + 3, PUBKEY_HASH_LEN);
    if (!buffer_seek_cur(&buf, script_len)) {
        THROW(SW_TX_PARSING_FAIL);
    }

    // size of extracted data
    return buf.offset;
}
