#pragma once

#include <stdbool.h>  // bool

/**
 * Action to take on user interaction on GET_XPUB call.
 *
 * @param[in] choice
 *   Wether user has confirmed.
 *
 */
void ui_action_confirm_xpub(bool choice);

/**
 * Action to take on GET_XPUB call
 *
 * @param[in] choice
 *   Wether user has confirmed.
 *
 */
void ui_action_confirm_address(bool choice);

void ui_action_sign_token_data(bool choice);

void ui_action_send_create_token(bool choice);
