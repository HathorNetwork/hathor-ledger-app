#ifndef STUBS_MOCK_ENGINE_H
#define STUBS_MOCK_ENGINE_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#define UNUSED(x) (void) x

#define MOCK_BUFFER_SIZE 10

#define MOCK_BOLOS_FN(fname, out_s)                                    \
    typedef struct out_s fname##_mock_resp_t;                          \
    extern fname##_mock_resp_t fname##_mock_returns[MOCK_BUFFER_SIZE]; \
    extern uint8_t fname##_mock_index;                                 \
    uint8_t get_##fname##_mock_index(void);                            \
    void fname##_mock_reset(void);                                     \
    void fname##_mock_add_response(uint8_t index, fname##_mock_resp_t resp);

#define MOCK_BOLOS(fname)                                                                    \
    fname##_mock_resp_t fname##_mock_returns[MOCK_BUFFER_SIZE];                              \
    uint8_t fname##_mock_index = 0;                                                          \
    uint8_t get_##fname##_mock_index(void) {                                                 \
        return fname##_mock_index++;                                                         \
    }                                                                                        \
    void fname##_mock_reset(void) {                                                          \
        fname##_mock_index = 0;                                                              \
        memset(fname##_mock_returns, 0, MOCK_BUFFER_SIZE * sizeof(fname##_mock_returns[0])); \
    }                                                                                        \
    void fname##_mock_add_response(uint8_t index, fname##_mock_resp_t resp) {                \
        memmove(&fname##_mock_returns[index], &resp, sizeof(resp));                          \
    }

#define MOCK_FN_INTERNAL_DEFS(fname)               \
    uint8_t get_index(void) {                      \
        return get_##fname##_mock_index();         \
    }                                              \
    fname##_mock_resp_t* get_resp(uint8_t index) { \
        return &fname##_mock_returns[index];       \
    }

#define MOCK_FN_INTERNAL(fname)                 \
    uint8_t index = get_##fname##_mock_index(); \
    fname##_mock_resp_t* response = &fname##_mock_returns[index];

#endif
