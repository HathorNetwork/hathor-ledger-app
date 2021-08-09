#include <stdbool.h>  // bool

#include "validate.h"
#include "../menu.h"
#include "../../sw.h"
#include "../../io.h"
#include "../../globals.h"
#include "../../helper/send_response.h"

void ui_action_confirm_xpub(bool choice) {
    if (choice) {
        helper_send_response_xpub();
    } else {
        explicit_bzero(&G_context, sizeof(G_context));
        io_send_sw(SW_DENY);
    }

    ui_menu_main();
}

void ui_action_confirm_address(bool choice) {
    if (choice) {
        io_send_sw(SW_OK);
    } else {
        explicit_bzero(&G_context, sizeof(G_context));
        io_send_sw(SW_DENY);
    }

    ui_menu_main();
}