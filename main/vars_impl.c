// ============================================================================
// EEZ Studio Native Variable Implementations
// ============================================================================
// Each native variable defined in EEZ Studio needs a getter (and optionally
// a setter) implemented here. EEZ generates the declarations in vars.h
// and wires them into the native_vars[] array in ui.c.
// ============================================================================

#include <stdint.h>
#include "beep.h"
#include "rtc_clock.h"

// --- beep_volume (integer, read/write) ---
int32_t get_var_beep_volume(void) {
    return (int32_t)beep_get_volume();
}

void set_var_beep_volume(int32_t value) {
    if (value < 0) value = 0;
    if (value > 100) value = 100;
    beep_set_volume((uint8_t)value);
}

// --- rtc_hours (integer, read-only) ---
int32_t get_var_rtc_hours(void) {
    return rtc_clock_get_hours();
}

void set_var_rtc_hours(int32_t value) {
    rtc_clock_set_time((uint8_t)value, (uint8_t)rtc_clock_get_minutes(), (uint8_t)rtc_clock_get_seconds());
}

// --- rtc_minutes (integer, read-only) ---
int32_t get_var_rtc_minutes(void) {
    return rtc_clock_get_minutes();
}

void set_var_rtc_minutes(int32_t value) {
    rtc_clock_set_time((uint8_t)rtc_clock_get_hours(), (uint8_t)value, (uint8_t)rtc_clock_get_seconds());
}

// --- rtc_seconds (integer, read-only) ---
int32_t get_var_rtc_seconds(void) {
    return rtc_clock_get_seconds();
}

void set_var_rtc_seconds(int32_t value) {
    (void)value;
}

// --- rtc_time_str (string, read-only) ---
const char *get_var_rtc_time_str(void) {
    return rtc_clock_get_time_str();
}

void set_var_rtc_time_str(const char *value) {
    (void)value;
}
