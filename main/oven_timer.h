#ifndef OVEN_TIMER_H
#define OVEN_TIMER_H

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

#define OVEN_TIMER_COUNT 2

typedef enum {
    OVEN_EVENT_TIMER1_EXPIRED,
    OVEN_EVENT_TIMER2_EXPIRED,
    OVEN_EVENT_DELAYED_START,
    OVEN_EVENT_COOKING_DONE,
} oven_event_t;

typedef void (*oven_event_cb_t)(oven_event_t event);

esp_err_t oven_timer_init(oven_event_cb_t on_event);

// --- Countdown timers (alarm beep only) ---
void oven_timer_set_hours(uint8_t id, int32_t hours);
void oven_timer_set_minutes(uint8_t id, int32_t minutes);
void oven_timer_set_seconds(uint8_t id, int32_t seconds);
int32_t oven_timer_get_hours(uint8_t id);
int32_t oven_timer_get_minutes(uint8_t id);
int32_t oven_timer_get_seconds(uint8_t id);
const char *oven_timer_get_str(uint8_t id);
void oven_timer_start(uint8_t id);
void oven_timer_stop(uint8_t id);
bool oven_timer_is_running(uint8_t id);

// --- Delayed start (triggers action at a target time of day) ---
void oven_delayed_start_set_hours(int32_t hours);
void oven_delayed_start_set_minutes(int32_t minutes);
int32_t oven_delayed_start_get_hours(void);
int32_t oven_delayed_start_get_minutes(void);
void oven_delayed_start_enable(void);
void oven_delayed_start_disable(void);
bool oven_delayed_start_is_enabled(void);

// --- Cooking time (countdown, triggers action when done) ---
void oven_cooking_set_hours(int32_t hours);
void oven_cooking_set_minutes(int32_t minutes);
int32_t oven_cooking_get_hours(void);
int32_t oven_cooking_get_minutes(void);
const char *oven_cooking_get_str(void);
void oven_cooking_start(void);
void oven_cooking_stop(void);
bool oven_cooking_is_running(void);

#ifdef __cplusplus
}
#endif

#endif
