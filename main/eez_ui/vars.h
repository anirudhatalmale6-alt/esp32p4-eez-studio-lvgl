#ifndef EEZ_LVGL_UI_VARS_H
#define EEZ_LVGL_UI_VARS_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// enum declarations

// Flow global variables

enum FlowGlobalVariables {
    FLOW_GLOBAL_VARIABLE_MAIN_TEMP_OVEN_1 = 0,
    FLOW_GLOBAL_VARIABLE_MAX_TEMP_OVEN_1 = 1,
    FLOW_GLOBAL_VARIABLE_DIGIT = 2,
    FLOW_GLOBAL_VARIABLE_MIN_AUDIO_VOLUME = 3,
    FLOW_GLOBAL_VARIABLE_MAX_AUDIO_VOLUME = 4,
    FLOW_GLOBAL_VARIABLE_SET_HOURS = 5,
    FLOW_GLOBAL_VARIABLE_SET_MINUTES = 6,
    FLOW_GLOBAL_VARIABLE_SET_SECONDES = 7,
    FLOW_GLOBAL_VARIABLE_LEFT_OVEN_MODE = 8,
    FLOW_GLOBAL_VARIABLE_DISPLAY_SET_TEMP_LEFT_OVEN = 9
};

// Native global variables

extern int32_t get_var_beep_volume();
extern void set_var_beep_volume(int32_t value);
extern int32_t get_var_rtc_hours();
extern void set_var_rtc_hours(int32_t value);
extern int32_t get_var_rtc_minutes();
extern void set_var_rtc_minutes(int32_t value);
extern int32_t get_var_rtc_seconds();
extern void set_var_rtc_seconds(int32_t value);
extern const char *get_var_rtc_time_str();
extern void set_var_rtc_time_str(const char *value);
extern int32_t get_var_timer1_hours();
extern void set_var_timer1_hours(int32_t value);
extern int32_t get_var_timer1_minutes();
extern void set_var_timer1_minutes(int32_t value);
extern int32_t get_var_timer1_seconds();
extern void set_var_timer1_seconds(int32_t value);
extern const char *get_var_timer1_str();
extern void set_var_timer1_str(const char *value);
extern int32_t get_var_timer1_running();
extern void set_var_timer1_running(int32_t value);
extern int32_t get_var_timer2_hours();
extern void set_var_timer2_hours(int32_t value);
extern int32_t get_var_timer2_minutes();
extern void set_var_timer2_minutes(int32_t value);
extern int32_t get_var_timer2_seconds();
extern void set_var_timer2_seconds(int32_t value);
extern const char *get_var_timer2_str();
extern void set_var_timer2_str(const char *value);
extern int32_t get_var_timer2_running();
extern void set_var_timer2_running(int32_t value);
extern int32_t get_var_backlight_brightness();
extern void set_var_backlight_brightness(int32_t value);

#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_VARS_H*/