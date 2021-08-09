#pragma once

#include "../types.h"

#define SIGN_TX_P1_DATA 0x00
#define SIGN_TX_P1_SIGN 0x01
#define SIGN_TX_P1_DONE 0x02

#define SIGN_TX_P2_SIGN 0x00
#define SIGN_TX_P2_DONE 0x00

#define SIGN_TX_P1_MAX 0x02

/**
 * Dispatch APDU command received to the right handler.
 *
 * @param[in] cmd
 *   Structured APDU command (CLA, INS, P1, P2, Lc, Command data).
 *
 * @return zero or positive integer if success, negative integer otherwise.
 *
 */
int apdu_dispatcher(const command_t *cmd);
