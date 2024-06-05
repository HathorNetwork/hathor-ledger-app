#pragma once

#include <stdbool.h>  // bool

/**
 * Callback to reuse action with approve/reject in step FLOW.
 */
typedef void (*action_validate_cb)(bool);

/**
 * Show user the parsed outputs to confirm
 * When all outputs are confirmed, show tx confirm flow
 */
int ui_display_tx_outputs(void);

/**
 * Ask the user to confirm sending the transaction
 * We enter a state where the caller can request the signature
 * of the received data upon confirmation
 */
int ui_display_tx_confirm(void);

/**
 * Ask the user permission to send the XPUB data on a given bip32 path
 */
int ui_display_xpub_confirm(void);

/**
 * Ask the user to confirm the address the app generates is the same as the wallet
 */
int ui_display_confirm_address(void);

int ui_display_reset_token_signatures_confirm(void);

int ui_display_sign_token_data(void);

int ui_display_create_token_data(void);
