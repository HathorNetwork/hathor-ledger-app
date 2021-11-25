#include "os.h"
#include "ux.h"
#include "glyphs.h"

#include "../globals.h"
#include "../storage.h"
#include "menu.h"

static char g_path_state[30];

UX_STEP_NOCB(ux_menu_ready_step, pnn, {&C_hathor_logo, "Hathor", "is ready"});
UX_STEP_NOCB(ux_menu_version_step, bn, {"Version", APPVERSION});
UX_STEP_CB(ux_menu_about_step, pb, ui_menu_about(), {&C_icon_certificate, "About"});
UX_STEP_CB(ux_menu_settings_step, pb, ui_menu_settings(), {&C_icon_coggle, "Settings"});
UX_STEP_VALID(ux_menu_exit_step, pb, os_sched_exit(-1), {&C_icon_dashboard_x, "Quit"});

// FLOW for the main menu:
// #1 screen: ready
// #2 screen: version of the app
// #3 screen: about submenu
// #4 screen: quit
UX_FLOW(ux_menu_main_flow,
        &ux_menu_ready_step,
        &ux_menu_version_step,
        &ux_menu_about_step,
        &ux_menu_settings_step,
        &ux_menu_exit_step,
        FLOW_LOOP);

void ui_menu_main() {
    if (G_ux.stack_count == 0) {
        ux_stack_push();
    }

    ux_flow_init(0, ux_menu_main_flow, NULL);
}

UX_STEP_NOCB(ux_menu_info_step, bn, {"Hathor App", "(c) 2021 Hathor Labs"});
UX_STEP_CB(ux_menu_back_step, pb, ui_menu_main(), {&C_icon_back, "Back"});

// FLOW for the about submenu:
// #1 screen: app info
// #2 screen: back button to main menu
UX_FLOW(ux_menu_about_flow, &ux_menu_info_step, &ux_menu_back_step, FLOW_LOOP);

void ui_menu_about() {
    ux_flow_init(0, ux_menu_about_flow, NULL);
}

void toggle_show_path_action() {
    toggle_show_path();

    // gimmick to redisplay show_path step with new settings;
    ui_menu_settings();
    ux_flow_next();
}

UX_STEP_NOCB(ux_menu_settings1_step, bn, {"Settings", ""});
UX_STEP_CB(ux_menu_settings_show_path_step,
           bn,
           toggle_show_path_action(),
           {"Show bip32 path", g_path_state});

// FLOW for the settings submenu:
// #1 screen: Settings
// #2 screen: toggle show_path settings
UX_FLOW(ux_menu_settings_flow,
        &ux_menu_settings1_step,
        &ux_menu_settings_show_path_step,
        &ux_menu_back_step,
        FLOW_LOOP);

void ui_menu_settings() {

    strcpy(g_path_state, N_storage.settings.show_path ? "on" : "off");

    ux_flow_init(0, ux_menu_settings_flow, NULL);
}
