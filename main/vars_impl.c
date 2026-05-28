#include <stdint.h>
#include "beep.h"
#include "backlight.h"
#include "rtc_clock.h"
#include "oven_timer.h"
#include "prizer_bus.h"

// === beep_volume (integer, read/write) ===
int32_t get_var_beep_volume(void) { return (int32_t)beep_get_volume(); }
void set_var_beep_volume(int32_t value) {
    if (value < 0) value = 0;
    if (value > 100) value = 100;
    beep_set_volume((uint8_t)value);
}

// === backlight_brightness (integer, read/write, 0-100%) ===
int32_t get_var_backlight_brightness(void) { return (int32_t)backlight_get_brightness(); }
void set_var_backlight_brightness(int32_t value) {
    if (value < 0) value = 0;
    if (value > 100) value = 100;
    backlight_set_brightness((uint8_t)value);
}

// === rtc_hours (integer, read/write) ===
int32_t get_var_rtc_hours(void) { return rtc_clock_get_hours(); }
void set_var_rtc_hours(int32_t value) {
    rtc_clock_set_time((uint8_t)value, (uint8_t)rtc_clock_get_minutes(), (uint8_t)rtc_clock_get_seconds());
}

// === rtc_minutes (integer, read/write) ===
int32_t get_var_rtc_minutes(void) { return rtc_clock_get_minutes(); }
void set_var_rtc_minutes(int32_t value) {
    rtc_clock_set_time((uint8_t)rtc_clock_get_hours(), (uint8_t)value, (uint8_t)rtc_clock_get_seconds());
}

// === rtc_seconds (integer, read-only) ===
int32_t get_var_rtc_seconds(void) { return rtc_clock_get_seconds(); }
void set_var_rtc_seconds(int32_t value) { (void)value; }

// === rtc_time_str (string, read-only) ===
const char *get_var_rtc_time_str(void) { return rtc_clock_get_time_str(); }
void set_var_rtc_time_str(const char *value) { (void)value; }

// === time_format (integer, read/write, 0=12h am/pm, 1=24h) ===
int32_t get_var_time_format(void) { return (int32_t)rtc_clock_get_format(); }
void set_var_time_format(int32_t value) { rtc_clock_set_format((uint8_t)value); }

// ===================== TIMER 1 =====================

int32_t get_var_timer1_hours(void) { return oven_timer_get_hours(0); }
void set_var_timer1_hours(int32_t value) { oven_timer_set_hours(0, value); }

int32_t get_var_timer1_minutes(void) { return oven_timer_get_minutes(0); }
void set_var_timer1_minutes(int32_t value) { oven_timer_set_minutes(0, value); }

int32_t get_var_timer1_seconds(void) { return oven_timer_get_seconds(0); }
void set_var_timer1_seconds(int32_t value) { oven_timer_set_seconds(0, value); }

const char *get_var_timer1_str(void) { return oven_timer_get_str(0); }
void set_var_timer1_str(const char *value) { (void)value; }

int32_t get_var_timer1_running(void) { return oven_timer_is_running(0) ? 1 : 0; }
void set_var_timer1_running(int32_t value) { (void)value; }

// ===================== TIMER 2 =====================

int32_t get_var_timer2_hours(void) { return oven_timer_get_hours(1); }
void set_var_timer2_hours(int32_t value) { oven_timer_set_hours(1, value); }

int32_t get_var_timer2_minutes(void) { return oven_timer_get_minutes(1); }
void set_var_timer2_minutes(int32_t value) { oven_timer_set_minutes(1, value); }

int32_t get_var_timer2_seconds(void) { return oven_timer_get_seconds(1); }
void set_var_timer2_seconds(int32_t value) { oven_timer_set_seconds(1, value); }

const char *get_var_timer2_str(void) { return oven_timer_get_str(1); }
void set_var_timer2_str(const char *value) { (void)value; }

int32_t get_var_timer2_running(void) { return oven_timer_is_running(1) ? 1 : 0; }
void set_var_timer2_running(int32_t value) { (void)value; }

// ===================== DELAYED START =====================

int32_t get_var_delayed_start_hours(void) { return oven_delayed_start_get_hours(); }
void set_var_delayed_start_hours(int32_t value) { oven_delayed_start_set_hours(value); }

int32_t get_var_delayed_start_minutes(void) { return oven_delayed_start_get_minutes(); }
void set_var_delayed_start_minutes(int32_t value) { oven_delayed_start_set_minutes(value); }

int32_t get_var_delayed_start_enabled(void) { return oven_delayed_start_is_enabled() ? 1 : 0; }
void set_var_delayed_start_enabled(int32_t value) { (void)value; }

// ===================== COOKING TIME =====================

int32_t get_var_cooking_hours(void) { return oven_cooking_get_hours(); }
void set_var_cooking_hours(int32_t value) { oven_cooking_set_hours(value); }

int32_t get_var_cooking_minutes(void) { return oven_cooking_get_minutes(); }
void set_var_cooking_minutes(int32_t value) { oven_cooking_set_minutes(value); }

const char *get_var_cooking_str(void) { return oven_cooking_get_str(); }
void set_var_cooking_str(const char *value) { (void)value; }

int32_t get_var_cooking_running(void) { return oven_cooking_is_running() ? 1 : 0; }
void set_var_cooking_running(int32_t value) { (void)value; }

// ===================== UART / PRIZER BUS =====================

int32_t get_var_setpoint_temp_oven1(void) { return (int32_t)prizer_get_setpoint_temp_oven1(); }
void set_var_setpoint_temp_oven1(int32_t value) { prizer_set_setpoint_temp_oven1((uint16_t)value); }

int32_t get_var_operating_mode_oven1(void) { return (int32_t)prizer_get_operating_mode_oven1(); }
void set_var_operating_mode_oven1(int32_t value) { prizer_set_operating_mode_oven1((uint8_t)value); }

int32_t get_var_setpoint_temp_oven2(void) { return (int32_t)prizer_get_setpoint_temp_oven2(); }
void set_var_setpoint_temp_oven2(int32_t value) { prizer_set_setpoint_temp_oven2((uint16_t)value); }

int32_t get_var_operating_mode_oven2(void) { return (int32_t)prizer_get_operating_mode_oven2(); }
void set_var_operating_mode_oven2(int32_t value) { prizer_set_operating_mode_oven2((uint8_t)value); }

int32_t get_var_uart_active(void) { return prizer_is_active() ? 1 : 0; }
void set_var_uart_active(int32_t value) { (void)value; }
