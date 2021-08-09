#pragma GCC diagnostic ignored "-Wformat-invalid-specifier"  // snprintf
#pragma GCC diagnostic ignored "-Wformat-extra-args"         // snprintf

#include <stdint.h>
#include <string.h>

#include "os.h"
#include "ux.h"
#include "glyphs.h"

#include "display.h"
#include "constants.h"
#include "../globals.h"
#include "../sw.h"
#include "action/validate.h"
#include "../common/format.h"
#include "menu.h"

#include "../hathor.h"

#include "../common/base58.h"

static action_validate_cb g_validate_callback;
static char g_amount[30];
static char g_bip32_path[60];
static char g_output_index[10];
static char g_address[B58_ADDRESS_LEN];

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

// Step with title/text for BIP32 path
UX_STEP_NOCB(ux_display_path_step,
             bnnn_paging,
             {
                 .title = "Path",
                 .text = g_bip32_path,
             });
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

/**
 * Prepare the UX screen values of the current output to confirm
 */
void prepare_display_output() {
    if (G_context.tx_info.has_change_output &&
        G_context.tx_info.confirmed_outputs == G_context.tx_info.change_output_index) {
        G_context.tx_info.display_index++;
        G_context.tx_info.confirmed_outputs++;
    }

    tx_output_t output = G_context.tx_info.outputs[G_context.tx_info.display_index];

    // set g_output_index
    uint8_t total_outputs = G_context.tx_info.outputs_len;
    uint8_t fake_output_index = output.index + 1;
    if (G_context.tx_info.has_change_output) {
        total_outputs--;
        if (output.index > G_context.tx_info.change_output_index) {
            fake_output_index--;
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
    strcpy(g_amount, "HTR ");
    format_fpu64(g_amount + 4, sizeof(g_amount) - 5, output.value, 2);
}

void ui_confirm_output(bool choice) {
    if (choice) {
        G_context.tx_info.display_index++;
        G_context.tx_info.confirmed_outputs++;
        if (G_context.tx_info.has_change_output &&
            G_context.tx_info.confirmed_outputs == G_context.tx_info.change_output_index) {
            G_context.tx_info.display_index++;
            G_context.tx_info.confirmed_outputs++;
        }
        if (G_context.tx_info.confirmed_outputs == G_context.tx_info.outputs_len) {
            // G_context.state = STATE_APPROVED;
            // io_send_sw(SW_OK);
            G_context.state = STATE_PARSED;
            ui_display_tx_confirm();
            return;
        }
        if (G_context.tx_info.display_index == G_context.tx_info.buffer_output_index) {
            G_context.tx_info.buffer_output_index = 0;
            G_context.tx_info.display_index = 0;
            io_send_sw(SW_OK);
            ui_menu_main();
            return;
        }
        ui_display_tx_outputs();
    } else {
        explicit_bzero(&G_context, sizeof(G_context));
        io_send_sw(SW_DENY);
        ui_menu_main();
    }
}

int ui_display_tx_outputs() {
    prepare_display_output();
    g_validate_callback = &ui_confirm_output;  // show next until need more
    ux_flow_init(0, ux_display_tx_output_flow, NULL);

    return 0;
}

// Get XPUB: ui_display_xpub_confirm

/* FLOW to display confirm access to XPUB:
 *  #1 screen: eye icon + "Confirm Access?"
 *  #2 screen: display BIP32 Path
 *  #3 screen: approve button
 *  #4 screen: reject button
 */
UX_FLOW(ux_display_xpub_flow,
        &ux_display_confirm_step,
        &ux_display_path_step,
        &ux_display_approve_step,
        &ux_display_reject_step,
        FLOW_LOOP);

int ui_display_xpub_confirm() {
    if (G_context.req_type != CONFIRM_XPUB || G_context.state != STATE_NONE) {
        G_context.state = STATE_NONE;
        return io_send_sw(SW_BAD_STATE);
    }

    memset(g_bip32_path, 0, sizeof(g_bip32_path));

    if (!bip32_path_format(G_context.bip32_path.path,
                           G_context.bip32_path.length,
                           g_bip32_path,
                           sizeof(g_bip32_path))) {
        return io_send_sw(SW_DISPLAY_BIP32_PATH_FAIL);
    }

    g_validate_callback = &ui_action_confirm_xpub;  // send xpub from bip32 path

    ux_flow_init(0, ux_display_xpub_flow, NULL);

    return 0;
}

// Get Address: ui_display_confirm_address

/* FLOW to display confirm address:
 *  #1 screen: eye icon + "Confirm Address?"
 *  #2 screen: display BIP32 Path
 *  #3 screen: display address
 *  #4 screen: approve button
 *  #5 screen: reject button
 */
UX_FLOW(ux_display_address_flow,
        &ux_display_confirm_addr_step,
        &ux_display_path_step,
        &ux_display_address_step,
        &ux_display_approve_step,
        &ux_display_reject_step,
        FLOW_LOOP);

int ui_display_confirm_address() {
    if (G_context.req_type != CONFIRM_ADDRESS || G_context.state != STATE_NONE) {
        G_context.state = STATE_NONE;
        return io_send_sw(SW_BAD_STATE);
    }

    memset(g_bip32_path, 0, sizeof(g_bip32_path));
    memset(g_address, 0, sizeof(g_address));

    if (!bip32_path_format(G_context.bip32_path.path,
                           G_context.bip32_path.length,
                           g_bip32_path,
                           sizeof(g_bip32_path))) {
        return io_send_sw(SW_DISPLAY_BIP32_PATH_FAIL);
    }

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
