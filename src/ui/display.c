#pragma GCC diagnostic ignored "-Wformat-invalid-specifier"  // snprintf
#pragma GCC diagnostic ignored "-Wformat-extra-args"         // snprintf

#include <stdint.h>
#include <string.h>

#include "ux.h"
#include "cx.h"

#include "glyphs.h"

#include "../common/base58.h"
#include "../common/format.h"
#include "../constants.h"
#include "../globals.h"
#include "../hathor.h"
#include "../storage.h"
#include "../sw.h"
#include "action/validate.h"
#include "display.h"
#include "menu.h"

static action_validate_cb g_validate_callback;
static char g_amount[30];
static char g_output_index[10];
static char g_address[B58_ADDRESS_LEN];
static char g_token_symbol[6];
static char g_token_name[31];
static char g_token_uid[65];
#ifdef UI_SHOW_PATH
static char g_bip32_path[60];

// Step with title/text for BIP32 path
UX_STEP_NOCB(ux_display_path_step,
             bnnn_paging,
             {
                 .title = "Path",
                 .text = g_bip32_path,
             });
#endif

/**
 * Clean context and return to menu.
 */
void action_exit_to_menu() {
    explicit_bzero(&G_context, sizeof(G_context));
    ui_menu_main();
}

UX_STEP_NOCB(ux_display_processing_step,
             pb,
             {
                 &C_icon_processing,
                 "Processing",
             });

UX_STEP_VALID(ux_sendtx_exit_step, pb, action_exit_to_menu(), {&C_icon_crossmark, "Quit"});

// Step with title/text for address
UX_STEP_NOCB(ux_display_address_step,
             bnnn_paging,
             {
                 .title = "Address",
                 .text = g_address,
             });
// Step with title/text for amount
UX_STEP_NOCB(ux_display_amount_step,
             bnnn_paging,
             {
                 .title = "Amount",
                 .text = g_amount,
             });
// Step with approve button
UX_STEP_CB(ux_display_approve_step,
           pb,
           (*g_validate_callback)(true),
           {
               &C_icon_validate_14,
               "Approve",
           });
// Step with reject button
UX_STEP_CB(ux_display_reject_step,
           pb,
           (*g_validate_callback)(false),
           {
               &C_icon_crossmark,
               "Reject",
           });

UX_STEP_NOCB(ux_display_confirm_addr_step, pnn, {&C_icon_eye, "Confirm", "Address"});

UX_STEP_NOCB(ux_display_review_output_step,
             pnn,
             {
                 &C_icon_eye,
                 "Output",
                 g_output_index,
             });

UX_STEP_NOCB(ux_display_confirm_sign_step,
             pnn,
             {
                 &C_icon_eye,
                 "Send",
                 "Transaction?",
             });

UX_STEP_NOCB(ux_display_confirm_step,
             pnn,
             {
                 &C_icon_eye,
                 "Confirm",
                 "access?",
             });

UX_STEP_NOCB(ux_display_reset_token_signatures_alert,
             pn,
             {
                 &C_icon_eye,
                 "Reset token signatures",
             });

UX_STEP_NOCB(ux_display_reset_token_signatures_warning,
             bnnn_paging,
             {
                 .title = "Warning",
                 .text = "This action will reset all token signatures",
             });

UX_STEP_NOCB(ux_display_token_data_0,
             pn,
             {
                 &C_icon_eye,
                 "Confirm token data",
             });

UX_STEP_NOCB(ux_display_token_data_1_symbol,
             bnnn_paging,
             {
                 .title = "Symbol",
                 .text = g_token_symbol,
             });

UX_STEP_NOCB(ux_display_token_data_2_name,
             bnnn_paging,
             {
                 .title = "Name",
                 .text = g_token_name,
             });

UX_STEP_NOCB(ux_display_token_data_3_uid,
             bnnn_paging,
             {
                 .title = "UID",
                 .text = g_token_uid,
             });

// Display a "Processing" message and allow user to stop processing and quit to menu
UX_FLOW(ux_display_processing, &ux_display_processing_step, &ux_sendtx_exit_step);

/**
 * User confirms to sign tx
 * we enter an approved state where the caller can request the signature of the received data
 *
 * @param[in]  choice
 *   A boolean representing wether the user confirmed or not.
 */
void ui_action_tx_confirm(bool choice) {
    if (choice) {
        G_context.state = STATE_APPROVED;
        io_send_sw(SW_OK);
        ux_flow_init(0, ux_display_processing, NULL);
        return;
    } else {
        explicit_bzero(&G_context, sizeof(G_context));
        io_send_sw(SW_DENY);
    }

    ui_menu_main();
}

/* FLOW to display confirm sign tx:
 *  #1 screen : eye icon + "Send Transaction?"
 *  #2 screen : approve button
 *  #3 screen : reject button
 */
UX_FLOW(ux_display_confirm_tx_flow,
        &ux_display_confirm_sign_step,
        &ux_display_approve_step,
        &ux_display_reject_step,
        FLOW_LOOP);

int ui_display_tx_confirm() {
    if (G_context.req_type != CONFIRM_TRANSACTION || G_context.state != STATE_PARSED) {
        explicit_bzero(&G_context, sizeof(G_context));
        io_send_sw(SW_BAD_STATE);
        ui_menu_main();
    } else {
        g_validate_callback = &ui_action_tx_confirm;  // set state_approved and send sw_ok
        ux_flow_init(0, ux_display_confirm_tx_flow, NULL);
    }

    return 0;
}

// SIGN_TX: confirm output
UX_FLOW(ux_display_tx_output_flow,
        &ux_display_review_output_step,  // Output <curr>/<total>
        &ux_display_address_step,        // address
        &ux_display_amount_step,         // HTR <value>
        &ux_display_approve_step,        // accept => decode next component and redisplay if needed
        &ux_display_reject_step,         // reject => return error
        FLOW_LOOP);

// Return true if we are showing something to the user
// The caller can use this as a signal to halt any other display processing
bool check_output_index_state() {
    // Check that we have reached the end of outputs array, show sign tx confirmation
    if (G_context.tx_info.confirmed_outputs == G_context.tx_info.outputs_len) {
        G_context.state = STATE_PARSED;
        ui_display_tx_confirm();
        return true;
    }
    // Check that we have showed all outputs on local buffer and request more data
    if (G_context.tx_info.display_index == G_context.tx_info.buffer_output_len) {
        G_context.tx_info.buffer_output_len = 0;
        G_context.tx_info.display_index = 0;
        // request more data
        io_send_sw(SW_OK);
        ui_menu_main();
        return true;
    }
    return false;
}

void inplace_selection_sort(size_t len, uint8_t* list) {
    size_t i, j, position;
    uint8_t tmp;
    for (i = 0; i < (len - 1); i++) {
        position = i;
        for (j = i + 1; j < len; j++) {
            if (list[position] > list[j]) position = j;
        }
        if (position != i) {
            tmp = list[position];
            list[position] = list[i];
            list[i] = tmp;
        }
    }
}

bool skip_change_outputs() {
    if (G_context.tx_info.change_len == 0) {
        // no change to skip
        return false;
    }
    // confirmed outputs holds the true current output index
    uint8_t change_indices[1 + TX_MAX_TOKENS];
    for (uint8_t i = 0; i < G_context.tx_info.change_len; i++) {
        change_indices[i] = G_context.tx_info.change_info[i].index;
    }
    inplace_selection_sort((size_t) G_context.tx_info.change_len, change_indices);

    for (uint8_t i = 0; i < G_context.tx_info.change_len; i++) {
        if (G_context.tx_info.confirmed_outputs == change_indices[i]) {
            // we are on a change index
            G_context.tx_info.display_index++;
            G_context.tx_info.confirmed_outputs++;
            if (check_output_index_state()) return true;
        }
    }

    return false;
}

/**
 * Prepare the UX screen values of the current output to confirm
 */
bool prepare_display_output() {
    // Check we have confirmed all outputs before attempting to display
    if (check_output_index_state()) return true;
    // skip change outputs if we have any
    if (skip_change_outputs()) return true;

    tx_output_t output = G_context.tx_info.outputs[G_context.tx_info.display_index];

    // set g_output_index
    uint8_t total_outputs = G_context.tx_info.outputs_len;
    uint8_t fake_output_index = output.index + 1;
    if (G_context.tx_info.change_len != 0) {
        // Remove change outputs from total
        total_outputs -= G_context.tx_info.change_len;
        for (uint8_t i = 0; i < G_context.tx_info.change_len; i++) {
            // Decrease 1 for each change output behind current output
            if (output.index > G_context.tx_info.change_info[i].index) fake_output_index--;
        }
    }
    itoa(fake_output_index, g_output_index, 10);
    uint8_t len = strlen(g_output_index);
    g_output_index[len++] = '/';
    itoa(total_outputs, g_output_index + len, 10);

    // set g_address
    memset(g_address, 0, sizeof(g_address));
    char b58address[B58_ADDRESS_LEN] = {0};
    uint8_t address[ADDRESS_LEN] = {0};
    address_from_pubkey_hash(output.pubkey_hash, address);
    base58_encode(address, ADDRESS_LEN, b58address, B58_ADDRESS_LEN);
    memmove(g_address, b58address, sizeof(b58address));

    // set g_ammount (HTR value)
    memset(g_amount, 0, sizeof(g_amount));
    int8_t token_index = output.token_data & TOKEN_DATA_INDEX_MASK;
    char symbol[MAX_TOKEN_SYMBOL_LEN + 1];
    uint8_t symbol_len;

    // token_index == 0 means HTR, else use token_index-1 as index on the tokens array
    if (token_index == 0) {
        strcpy(symbol, "HTR");
        symbol_len = 3;
    } else {
        // custom token
        token_symbol_t* token = G_context.tx_info.tokens[token_index - 1];
        strcpy(symbol, token->symbol);
        symbol_len = strlen(token->symbol);
    }
    strcpy(g_amount, symbol);
    g_amount[symbol_len] = ' ';
    format_value(output.value, g_amount + symbol_len + 1);
    return false;
}

void ui_confirm_output(bool choice) {
    if (choice) {
        G_context.tx_info.display_index++;
        G_context.tx_info.confirmed_outputs++;
        // return if we are requesting more data or we have confirmed all outputs
        if (skip_change_outputs()) return;
        // Show next output from buffer
        ui_display_tx_outputs();
    } else {
        explicit_bzero(&G_context, sizeof(G_context));
        io_send_sw(SW_DENY);
        ui_menu_main();
    }
}

int ui_display_tx_outputs() {
    // skip changes, return ok if there is no more on buffer
    if (prepare_display_output()) return 0;
    g_validate_callback = &ui_confirm_output;  // show next until need more
    ux_flow_init(0, ux_display_tx_output_flow, NULL);

    return 0;
}

// Get XPUB: ui_display_xpub_confirm

/* FLOW to display confirm access to XPUB:
 *  #1 screen: eye icon + "Confirm Access?"
 *  #2 screen: display BIP32 Path (if enabled)
 *  #3 screen: approve button
 *  #4 screen: reject button
 */
UX_FLOW(ux_display_xpub_flow,
        &ux_display_confirm_step,
#ifdef UI_SHOW_PATH
        &ux_display_path_step,
#endif
        &ux_display_approve_step,
        &ux_display_reject_step,
        FLOW_LOOP);

int ui_display_xpub_confirm() {
    if (G_context.req_type != CONFIRM_XPUB || G_context.state != STATE_NONE) {
        G_context.state = STATE_NONE;
        return io_send_sw(SW_BAD_STATE);
    }

#ifdef UI_SHOW_PATH
    memset(g_bip32_path, 0, sizeof(g_bip32_path));

    if (!bip32_path_format(G_context.bip32_path.path,
                           G_context.bip32_path.length,
                           g_bip32_path,
                           sizeof(g_bip32_path))) {
        return io_send_sw(SW_DISPLAY_BIP32_PATH_FAIL);
    }
#endif

    g_validate_callback = &ui_action_confirm_xpub;  // send xpub from bip32 path

    ux_flow_init(0, ux_display_xpub_flow, NULL);

    return 0;
}

// Get Address: ui_display_confirm_address

/* FLOW to display confirm address:
 *  #1 screen: eye icon + "Confirm Address?"
 *  #2 screen: display BIP32 Path (if enabled)
 *  #3 screen: display address
 *  #4 screen: approve button
 */
UX_FLOW(ux_display_address_flow,
        &ux_display_confirm_addr_step,
#ifdef UI_SHOW_PATH
        &ux_display_path_step,
#endif
        &ux_display_address_step,
        &ux_display_approve_step,
        FLOW_LOOP);

int ui_display_confirm_address() {
    if (G_context.req_type != CONFIRM_ADDRESS || G_context.state != STATE_NONE) {
        G_context.state = STATE_NONE;
        return io_send_sw(SW_BAD_STATE);
    }

    memset(g_address, 0, sizeof(g_address));

#ifdef UI_SHOW_PATH
    memset(g_bip32_path, 0, sizeof(g_bip32_path));
    if (!bip32_path_format(G_context.bip32_path.path,
                           G_context.bip32_path.length,
                           g_bip32_path,
                           sizeof(g_bip32_path))) {
        return io_send_sw(SW_DISPLAY_BIP32_PATH_FAIL);
    }
#endif

    cx_ecfp_private_key_t private_key = {0};
    cx_ecfp_public_key_t public_key = {0};
    uint8_t chain_code[32];

    uint8_t address[ADDRESS_LEN] = {0};
    char b58address[B58_ADDRESS_LEN] = {0};

    // derive for bip32 path
    derive_private_key(&private_key,
                       chain_code,
                       G_context.bip32_path.path,
                       G_context.bip32_path.length);
    init_public_key(&private_key, &public_key);

    // Generate address from public key
    address_from_pubkey(&public_key, address);
    base58_encode(address, ADDRESS_LEN, b58address, B58_ADDRESS_LEN);
    memmove(g_address, b58address, sizeof(b58address));

    explicit_bzero(&private_key, sizeof(private_key));
    explicit_bzero(&public_key, sizeof(public_key));

    g_validate_callback = &ui_action_confirm_address;

    ux_flow_init(0, ux_display_address_flow, NULL);

    return 0;
}

// Reset token signatures: ui_display_confirm_address

void ui_confirm_reset_token_signatures(bool choice) {
    if (choice) {
        // generates and saves new secret
        generate_secret();
        io_send_sw(SW_OK);
    } else {
        // return error, denied by user
        io_send_sw(SW_DENY);
    }
    ui_menu_main();
}

/* FLOW to display confirm address:
 *  #1 screen: eye icon + "Reset token signatures"
 *  #2 screen: warning message
 *  #3 screen: approve button
 *  #4 screen: reject button
 */
UX_FLOW(ux_display_reset_token_signatures,
        &ux_display_reset_token_signatures_alert,
        &ux_display_reset_token_signatures_warning,
        &ux_display_approve_step,
        &ux_display_reject_step,
        FLOW_LOOP);

int ui_display_reset_token_signatures_confirm() {
    g_validate_callback = &ui_confirm_reset_token_signatures;
    ux_flow_init(0, ux_display_reset_token_signatures, NULL);
    return 0;
}

/* FLOW to sign token data:
 *  #1 screen: eye icon + "Confirm token data"
 *  #2 screen: symbol
 *  #2 screen: name
 *  #2 screen: uid
 *  #3 screen: approve button
 *  #4 screen: reject button
 */
UX_FLOW(ux_display_sign_token_data,
        &ux_display_token_data_0,
        &ux_display_token_data_1_symbol,
        &ux_display_token_data_2_name,
        &ux_display_token_data_3_uid,
        &ux_display_approve_step,
        &ux_display_reject_step,
        FLOW_LOOP);

int ui_display_sign_token_data() {
    // show token information
    // copy symbol
    memmove(g_token_symbol, G_context.token.symbol, G_context.token.symbol_len);
    g_token_symbol[G_context.token.symbol_len] = '\0';
    // copy name
    memmove(g_token_name, G_context.token.name, G_context.token.name_len);
    g_token_name[G_context.token.name_len] = '\0';
    // format uid
    format_hex(G_context.token.uid, TOKEN_UID_LEN, g_token_uid, 65);
    // ask confirmation to sign
    g_validate_callback = &ui_action_sign_token_data;
    ux_flow_init(0, ux_display_sign_token_data, NULL);
    return 0;
}
