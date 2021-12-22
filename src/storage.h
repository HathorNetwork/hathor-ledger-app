#pragma once

#include "globals.h"

/**
 * Get N_storage.secret.
 * If it's the first call ever, generate and save a secret.
 *
 * @param[out] secret
 *
 */
void get_secret(uint8_t* secret);

/**
 * Generate new random secret and write to N_storage.secret.
 */
void generate_secret(void);
