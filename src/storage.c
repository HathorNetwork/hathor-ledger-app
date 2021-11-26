#include "storage.h"

uint32_t get_secret() {
    if (N_storage.secret == 0) {
        // first access, generate and write to storage
        return generate_secret();
    }
    return (volatile uint32_t) N_storage.secret;
}

uint32_t generate_secret() {
    uint32_t new_secret = cx_rng_u32();
    nvm_write((void *) &N_storage.secret, &new_secret, sizeof(uint32_t));
    return new_secret;
}
