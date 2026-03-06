#if defined(EEZ_FOR_LVGL)
#include <eez/core/vars.h>
#endif

#include "ui.h"
#include "screens.h"
#include "images.h"
#include "actions.h"
#include "vars.h"

// ============================================================================
// PLACEHOLDER — This file will be replaced by EEZ Studio generated code.
// The assets[] array below is a minimal empty EEZ asset blob.
// When you export from EEZ Studio, it generates this file with the real
// binary assets containing your flow definitions, screen layouts, etc.
// ============================================================================

// Minimal EEZ asset blob (empty project, just enough for flow engine init)
const uint8_t assets[36] = {
    0x7E, 0x45, 0x45, 0x5A, // Magic: ~EEZ
    0x03, 0x00,             // Version: 3
    0x00, 0x00,             // Flags
    0x00, 0x00, 0x00, 0x00, // Reserved
    0x24, 0x00, 0x00, 0x00, // Header size
    0x24, 0x00, 0x00, 0x00, // Total size
    0x00, 0x00, 0x00, 0x00, // Reserved
    0x00, 0x00, 0x00, 0x00, // Reserved
    0x00, 0x00, 0x00, 0x00, // Reserved
    0x00, 0x00, 0x00, 0x00, // Reserved
};

native_var_t native_vars[] = {
    { NATIVE_VAR_TYPE_NONE, 0, 0 },
};

ActionExecFunc actions[] = {
    NULL, // Placeholder — EEZ Studio will generate action function pointers here
};

#if defined(EEZ_FOR_LVGL)

void ui_init() {
    eez_flow_init(assets, sizeof(assets), (lv_obj_t **)&objects, sizeof(objects),
                  images, sizeof(images), actions);
    init_vars();
}

void ui_tick() {
    eez_flow_tick();
    tick_vars();
    tick_screen(g_currentScreen);
}

#else

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

#endif
