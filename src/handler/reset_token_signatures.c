#include "reset_token_signatures.h"
#include "../ui/display.h"

int handler_reset_token_signatures() {
    return ui_display_reset_token_signatures_confirm();
}
