#pragma once

#include "globals.h"

/**
 * Get N_storage.secret.
 * If it's the first call ever, generate and save a secret before return.
 *
 * @returns uint32_t the internal secret seed;
 */
uint32_t get_secret(void);

/**
 * Generate new random secret and write to N_storage.secret.
 *
 * @returns uint32_t the new internal secret seed;
 */
uint32_t generate_secret(void);
