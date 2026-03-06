#include "ui.h"
#include "screens.h"
#include "images.h"
#include "actions.h"
#include "vars.h"

// ============================================================================
// PLACEHOLDER — This file will be REPLACED by EEZ Studio generated code.
//
// This placeholder uses direct LVGL screen creation (no EEZ Flow engine).
// When you export from EEZ Studio, it generates a real ui.c with a valid
// assets[] blob and eez_flow_init(). The import script will replace this file.
// ============================================================================

static int16_t currentScreen = -1;

void ui_init() {
    create_screens();
    currentScreen = 0;
    lv_obj_t *screen = objects.main;
    lv_scr_load(screen);
}

void ui_tick() {
    if (currentScreen >= 0) {
        tick_screen(currentScreen);
    }
}
