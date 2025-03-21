#include <stdbool.h>  // bool
#include <string.h>   // memmove

#include "deserialize.h"

#include "os.h"  // THROW

#include "../sw.h"
#include "constants.h"
#include "../common/buffer.h"
#include "types.h"
#include "script.h"

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

    // parse script
    uint16_t err = parse_output_script(&buf, script_len, &output->script);
    if (err == ERR_MORE_DATA_REQUIRED) {
        // More data is required to parse the script
        THROW(TX_STATE_READY);
    }
    if (err) {
        PRINTF("Error parsing output script: %d\n", err);
        THROW(err);
    }

    if (!buffer_seek_cur(&buf, script_len)) {
        THROW(SW_TX_PARSING_FAIL);
    }

    // size of extracted data
    return buf.offset;
}
