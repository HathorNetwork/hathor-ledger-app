#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <stdint.h>
#include <stdbool.h>

#include <cmocka.h>

#include "common/bip32.h"
#include "common/read.h"

static void test_bip32_format(void **state) {
    (void) state;

    char output[30];
    bool b = false;

    b = bip32_path_format((const uint32_t[5]){0x8000002C, 0x80000000, 0x80000000, 0, 0},
                          5,
                          output,
                          sizeof(output));
    assert_true(b);
    assert_string_equal(output, "44'/0'/0'/0/0");

    b = bip32_path_format((const uint32_t[5]){0x8000002C, 0x80000001, 0x80000000, 0, 0},
                          5,
                          output,
                          sizeof(output));
    assert_true(b);
    assert_string_equal(output, "44'/1'/0'/0/0");
}

static void test_bad_bip32_format(void **state) {
    (void) state;

    char output[30];
    bool b = true;

    // More than MAX_BIP32_PATH (=10)
    b = bip32_path_format(
        (const uint32_t[11]){0x8000002C, 0x80000000, 0x80000000, 0, 0, 0, 0, 0, 0, 0, 0},
        11,
        output,
        sizeof(output));
    assert_false(b);

    // No BIP32 path (=0)
    b = bip32_path_format(NULL, 0, output, sizeof(output));
    assert_false(b);
}

static void test_bip32_read(void **state) {
    (void) state;

    // clang-format off
    uint8_t input[21] = {
        0x05, // length
        0x80, 0x00, 0x00, 0x2C,
        0x80, 0x00, 0x01, 0x18,
        0x80, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x0a
    };
    uint32_t expected[5] = {0x8000002C, 0x80000118, 0x80000000, 0, 10};
    bip32_path_t path_out = { .path = {0}, .length = 0 };
    bool b = false;

    b = bip32_path_read(input, sizeof(input), &path_out);
    assert_true(b);
    assert_int_equal(path_out.length, 5);
    assert_memory_equal(path_out.path, expected, 5);
}

static void test_bad_bip32_read(void **state) {
    (void) state;

    bip32_path_t path_out = { .path = {0}, .length = 0 };

    // clang-format off
    uint8_t input[5] = {
        0x02,                   // length 2 (but buffer only has 1)
        0x80, 0x00, 0x00, 0x2C, // 44'
    };

    // buffer too small (5 BIP32 paths instead of 10)
    assert_false(bip32_path_read(input, sizeof(input), &path_out));

    // changed the input here to not collide with previous tests
    // clang-format off
    uint8_t input2[25] = {
        0x06,                   // length 6 when max permitted is 5
        0x80, 0x00, 0x00, 0x2C, // 44'
        0x80, 0x00, 0x00, 0x01, // 1'
        0x80, 0x00, 0x00, 0x00, // 0'
        0x00, 0x00, 0x00, 0x00, // 0
        0x00, 0x00, 0x00, 0x00, // 0
        0x00, 0x00, 0x00, 0x00  // 0
    };

    // More than MAX_BIP32_PATH (=5)
    assert_false(bip32_path_read(input2, sizeof(input2), &path_out));
}

int main() {
    const struct CMUnitTest tests[] = {cmocka_unit_test(test_bip32_format),
                                       cmocka_unit_test(test_bad_bip32_format),
                                       cmocka_unit_test(test_bip32_read),
                                       cmocka_unit_test(test_bad_bip32_read)};

    return cmocka_run_group_tests(tests, NULL, NULL);
}
