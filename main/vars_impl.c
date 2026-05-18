// ============================================================================
// EEZ Studio Native Variable Implementations
// ============================================================================
// In EEZ Studio, add a Native Variable named "beep_volume" (type: integer).
// EEZ will generate getter/setter declarations in vars.h and wire them
// into the native_vars[] array in ui.c.
//
// This file provides the actual getter/setter implementations.
// ============================================================================

#include <stdint.h>
#include "beep.h"

int32_t get_var_beep_volume(void) {
    return (int32_t)beep_get_volume();
}

void set_var_beep_volume(int32_t value) {
    if (value < 0) value = 0;
    if (value > 100) value = 100;
    beep_set_volume((uint8_t)value);
}
