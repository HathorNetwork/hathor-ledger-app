#include <stddef.h>   // size_t
#include <stdint.h>   // int*_t, uint*_t
#include <string.h>   // strncpy, memmove
#include <stdbool.h>  // bool

#include "format.h"

bool format_i64(char *dst, size_t dst_len, const int64_t value) {
    char temp[] = "-9223372036854775808";

    // Check for null pointers
    if ((dst == NULL) || (dst_len == 0)) {
        return false;
    }

    char *ptr = temp;
    int64_t num = value;
    int sign = 1;

    if (value < 0) {
        sign = -1;
    }

    while (num != 0) {
        *ptr++ = '0' + (num % 10) * sign;
        num /= 10;
    }

    if (value < 0) {
        *ptr++ = '-';
    } else if (value == 0) {
        *ptr++ = '0';
    }

    int distance = (ptr - temp) + 1;

    if ((int) dst_len < distance) {
        return false;
    }

    size_t index = 0;

    while (--ptr >= temp) {
        dst[index++] = *ptr;
    }

    dst[index] = '\0';

    return true;
}

bool format_u64(char *out, size_t outLen, uint64_t in) {
    uint8_t i = 0;

    if ((out == NULL) || (outLen == 0)) {
        return false;
    }
    outLen--;

    while (in > 9) {
        out[i] = in % 10 + '0';
        in /= 10;
        i++;
        if (i + 1 > outLen) {
            return false;
        }
    }
    out[i] = in + '0';
    out[i + 1] = '\0';

    uint8_t j = 0;
    char tmp;

    // revert the string
    while (j < i) {
        // swap out[j] and out[i]
        tmp = out[j];
        out[j] = out[i];
        out[i] = tmp;

        i--;
        j++;
    }
    return true;
}

bool format_fpu64(char *dst, size_t dst_len, const uint64_t value, uint8_t decimals) {
    char buffer[21] = {0};

    if (!format_u64(buffer, sizeof(buffer), value)) {
        return false;
    }

    size_t digits = strlen(buffer);

    if (digits <= decimals) {
        if (dst_len <= 2 + decimals - digits) {
            return false;
        }
        *dst++ = '0';
        *dst++ = '.';
        for (uint16_t i = 0; i < decimals - digits; i++, dst++) {
            *dst = '0';
        }
        dst_len -= 2 + decimals - digits;
        strncpy(dst, buffer, dst_len);
    } else {
        if (dst_len <= digits + 1 + decimals) {
            return false;
        }

        const size_t shift = digits - decimals;
        memmove(dst, buffer, shift);
        dst[shift] = '.';
        strncpy(dst + shift + 1, buffer + shift, decimals);
    }

    return true;
}

int format_hex(const uint8_t *in, size_t in_len, char *out, size_t out_len) {
    if (out_len < 2 * in_len + 1) {
        return -1;
    }

    if ((out == NULL) || (in == NULL)) {
        return -1;
    }

    const char hex[] = "0123456789ABCDEF";
    int written = 0;
    uint8_t high_nibble, low_nibble;

    for (size_t i = 0; i < in_len; i++) {
        high_nibble = (in[i] & 0xF0) >> 4;
        low_nibble = in[i] & 0x0F;

        out[written++] = hex[high_nibble];
        out[written++] = hex[low_nibble];
    }

    out[written++] = '\0';

    return written;
}

void format_value(uint64_t value, char *out) {
    // first deal with the part to the left of the decimal separator
    uint64_t tmp = value / 100;
    int c;
    char buf[35];
    char *p;

    format_u64(buf, sizeof(buf), tmp);
    // 'c' is used here to control when a comma should be added
    c = 2 - strlen(buf) % 3;
    for (p = buf; *p != 0; p++) {
        *out++ = *p;
        if (c == 1) {
            *out++ = ',';
        }
        c = (c + 1) % 3;
    }
    *--out = 0;

    // now the part to the right of the decimal separator
    tmp = value % 100;
    // reusing 'c' variable to point to the end of string, to add decimal
    // separator and possibly pad with 0
    c = strlen((const char *) out);
    out[c++] = '.';
    if (tmp < 10) {
        out[c++] = '0';
    }
    itoa(tmp, (char *) out + c, 10);
}

void itoa(int value, char *result, int base) {
    if (result == NULL) {
        return;
    }

    // check that the base if valid
    if (base < 2 || base > 36) {
        *result = '\0';
    }

    char *ptr = result, *ptr1 = result, tmp_char;
    int tmp_value;

    do {
        tmp_value = value;
        value /= base;
        *ptr++ = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz"
            [35 + (tmp_value - value * base)];
    } while (value);

    // Apply negative sign
    if (tmp_value < 0) *ptr++ = '-';
    *ptr-- = '\0';
    while (ptr1 < ptr) {
        tmp_char = *ptr;
        *ptr-- = *ptr1;
        *ptr1++ = tmp_char;
    }
}

bool is_printable(char *str, int len) {
    // this method works on char arrays and strings
    for (int i = 0; i < len; i++) {
        // to catch the end of a string
        if (str[i] == '\0') return true;
        uint8_t c = (uint8_t) str[i];
        // printable ascii characters 0x20-0x7f
        if (c < 0x20u || c >= 0x80u) return false;
    }
    // char arrays (not strings) do not end with '\0'
    return true;
}
