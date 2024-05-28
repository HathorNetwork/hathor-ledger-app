#include <stdbool.h>  // bool
#include <string.h>   // memset, explicit_bzero, memmove

#include "os.h"  // PRINTF

#include "script.h"
#include "../constants.h"
#include "opcodes.h"

/**
 * Identifies if the given script is P2PKH
 */
bool identify_p2pkh_script(buffer_t *in, uint16_t script_len) {
    uint8_t p2pkh[] = {OP_DUP, OP_HASH160, PUBKEY_HASH_LEN, OP_EQUALVERIFY, OP_CHECKSIG};
    size_t offset = 0;

    if ((in == NULL) || ((script_len != 25) && (script_len != 31)) ||
        (in->size - in->offset < script_len)) {
        PRINTF("[-] [Identify P2PKH script] invalid args (script_len=%d, size=%d, offset=%d)\n",
               script_len,
               in->size,
               in->offset);
        return false;
    }

    if (script_len == 31) {
        // check timelock
        if (in->ptr[in->offset] != 4) {
            PRINTF("[-] [Identify P2PKH script] invalid timelock: more than 4 bytes\n");
            return false;
        }
        // skip push_4 (1 byte) + timestamp (4 bytes)
        offset += 5;
        if (in->ptr[in->offset + offset] != OP_GREATERTHAN_TIMESTAMP) {
            PRINTF(
                "[-] [Identify P2PKH script] invalid timelock: OP_GREATERTHAN_TIMESTAMP missing\n");
            return false;
        }
        // skip OP_GREATERTHAN_TIMESTAMP (1 byte)
        offset += 1;
    }

    // check OP_DUP, OP_HASH160, push_20
    if (memcmp(p2pkh, in->ptr + in->offset + offset, 3) != 0) {
        PRINTF("[-] [Identify P2PKH script] invalid script: starting opcodes mismatch\n");
        return false;
    }
    // Skip opcodes and pubkey hash
    offset += 3 + PUBKEY_HASH_LEN;

    // check OP_EQUALVERIFY, OP_CHECKSIG
    if (memcmp(p2pkh + 3, in->ptr + in->offset + offset, 2) != 0) {
        PRINTF("[-] [Identify P2PKH script] invalid script: ending opcodes mismatch\n");
        return false;
    }

    return true;
}

/**
 * Identifies if the given script is P2SH
 */
bool identify_p2sh_script(buffer_t *in, uint16_t script_len) {
    uint8_t p2sh[] = {OP_HASH160, PUBKEY_HASH_LEN, OP_EQUAL};
    size_t offset = 0;

    if ((in == NULL) || ((script_len != 23) && (script_len != 29)) ||
        (in->size - in->offset < script_len)) {
        PRINTF("[-] [Identify P2SH script] invalid args (script_len=%d, size=%d, offset=%d)\n",
               script_len,
               in->size,
               in->offset);
        return false;
    }

    if (script_len == 29) {
        // check timelock
        if (in->ptr[in->offset] != 4) {
            PRINTF("[-] [Identify P2SH script] invalid timelock: more than 4 bytes\n");
            return false;
        }
        // skip push_4 (1 byte) + timestamp (4 bytes)
        offset += 5;
        if (in->ptr[in->offset + offset] != OP_GREATERTHAN_TIMESTAMP) {
            PRINTF(
                "[-] [Identify P2SH script] invalid timelock: OP_GREATERTHAN_TIMESTAMP missing\n");
            return false;
        }
        // skip OP_GREATERTHAN_TIMESTAMP (1 byte)
        offset += 1;
    }

    // check OP_HASH160, push_20
    if (memcmp(p2sh, in->ptr + in->offset + offset, 2) != 0) {
        PRINTF("[-] [Identify P2SH script] invalid script: starting opcodes mismatch\n");
        return false;
    }
    // Skip opcodes and script hash
    offset += 2 + PUBKEY_HASH_LEN;

    // check OP_EQUAL
    if (in->ptr[in->offset + offset] != OP_EQUAL) {
        PRINTF("[-] [Identify P2SH script] invalid script: ending opcode mismatch\n");
        return false;
    }

    return true;
}

bool identify_data_script(buffer_t *in, uint16_t script_len) {
    // Initial expected_len is 2 due to obligatory data push + OP_CHECKSIG
    uint16_t expected_len = 2;
    uint8_t data_len = 0;
    if ((in == NULL) || (script_len < expected_len) || (script_len > MAX_DATA_SCRIPT_LEN) ||
        (in->size - in->offset < script_len)) {
        PRINTF("[-] [Identify data script] invalid args (script_len=%d, size=%d, offset=%d)\n",
               script_len,
               in->size,
               in->offset);
        return false;
    }
    if (in->ptr[in->offset] == OP_PUSHDATA1) {
        // If first opcode is OP_PUSHDATA1, the second is the data length
        // then we push the actual data
        // Valid for lengths [1,255] but used in (75,255] due to extra opcode byte
        // expected_len is increased due to the extra OP_PUSHDATA1
        expected_len += 1;
        data_len = in->ptr[in->offset + 1];
    } else {
        // The first opcode is the data length
        // then we push the actual data
        data_len = in->ptr[in->offset];
        if (data_len > 75) {
            // Invalid script
            // This case is only valid for [1,75] bytes of data
            PRINTF("[-] [Identify data script] invalid pushdata (pushdata=%d)\n", data_len);
            return false;
        }
    }
    expected_len += data_len;
    if (script_len != expected_len) {
        // Invalid script
        // Here we check for actual data push and data + OP_CHECKSIG length
        PRINTF("[-] [Identify data script] invalid script length (script_len=%d, expected=%d)\n",
               script_len,
               expected_len);
        return false;
    }

    // It should end with OP_CHECKSIG
    if (in->ptr[in->offset + script_len - 1] != OP_CHECKSIG) {
        PRINTF("[-] [Identify data script] invalid script: missing ending OP_CHECKSIG\n");
        return false;
    }

    return true;
}

/**
 * Read data from script and store it in `out`
 * Returns number of bytes read, 0 if any error occurs.
 */
uint8_t read_data_script(buffer_t *in, uint16_t script_len, uint8_t out[MAX_DATA_SCRIPT_LEN]) {
    // Initial expected_len is 2 due to obligatory data push + OP_CHECKSIG
    uint16_t expected_len = 2;
    uint8_t data_len = 0;
    size_t data_start = 0;
    if ((in == NULL) || (out == NULL) || (script_len < expected_len) ||
        (script_len > MAX_DATA_SCRIPT_LEN) || (in->size - in->offset < script_len)) {
        return 0;
    }
    if (in->ptr[in->offset] == OP_PUSHDATA1) {
        // If first opcode is OP_PUSHDATA1, the second is the data length
        // then we push the actual data
        // Valid for lengths [1,255] but used in (75,255] due to extra opcode byte
        // expected_len is increased due to the extra OP_PUSHDATA1
        expected_len += 1;
        data_len = in->ptr[in->offset + 1];
        data_start = 2;
    } else {
        // The first opcode is the data length
        // then we push the actual data
        // This case is only valid for [1,75] bytes of data
        data_len = in->ptr[in->offset];
        data_start = 1;
        if (data_len > 75) {
            // Invalid script
            // Maybe return error code?
            return 0;
        }
    }
    expected_len += data_len;
    if (script_len != expected_len) {
        // Invalid script
        // Here we check for actual data push and data + OP_CHECKSIG length
        return 0;
    }

    // Read actual data from buffer
    memmove(out, in->ptr + in->offset + data_start, data_len);
    return data_len;
}

/**
 * Identifies the given script type
 */
script_type_t identify_script(buffer_t *in, uint16_t script_len) {
    if (identify_p2pkh_script(in, script_len)) {
        return SCRIPT_P2PKH;
    } else if (identify_p2sh_script(in, script_len)) {
        return SCRIPT_P2SH;
    } else if (identify_data_script(in, script_len)) {
        return SCRIPT_DATA;
    } else {
        // Unsupported script should fail output parsing
        return SCRIPT_UNKNOWN;
    }
}

uint16_t parse_output_script(buffer_t *in, uint16_t script_len, output_script_info_t *out) {
    if (in == NULL || out == NULL) {
        // Choose an error code for invalid arguments
        return ERR_INVALID_ARGS;
    }

    if ((in->size - in->offset) < script_len) {
        // More data is required to parse the script
        return ERR_MORE_DATA_REQUIRED;
    }

    switch (identify_script(in, script_len)) {
        case SCRIPT_P2PKH:
            out->type = SCRIPT_P2PKH;
            if (script_len != 25) {
                // timelock is not supported yet
                return ERR_UNSUPPORTED_SCRIPT;
            }
            memmove(out->hash, in->ptr + in->offset + 3, PUBKEY_HASH_LEN);
            break;
        case SCRIPT_P2SH:
            out->type = SCRIPT_P2SH;
            if (script_len != 23) {
                // timelock is not supported yet
                return ERR_UNSUPPORTED_SCRIPT;
            }
            memmove(out->hash, in->ptr + in->offset + 2, PUBKEY_HASH_LEN);
            break;
        case SCRIPT_DATA:
            out->type = SCRIPT_DATA;
            // XXX: We currently do not read the actual data from the output
            break;
        case SCRIPT_UNKNOWN:
        default:
            return ERR_UNSUPPORTED_SCRIPT;
    }
    return 0;
}
