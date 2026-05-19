#ifndef OVEN_TIMER_H
#define OVEN_TIMER_H

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

#define OVEN_TIMER_COUNT 2

typedef void (*oven_timer_expired_cb_t)(uint8_t timer_id);

esp_err_t oven_timer_init(oven_timer_expired_cb_t on_expired);

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

int32_t oven_timer_get_remaining(uint8_t id);

#ifdef __cplusplus
}
#endif

#endif
