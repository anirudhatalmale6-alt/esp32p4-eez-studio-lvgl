#include "oven_timer.h"
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
#include "esp_log.h"

static const char *TAG = "timer";

typedef struct {
    int32_t preset_h;
    int32_t preset_m;
    int32_t preset_s;
    int32_t remaining_s;
    bool running;
    char str_buf[12];
} timer_state_t;

static timer_state_t timers[OVEN_TIMER_COUNT];
static TimerHandle_t tick_timer = NULL;
static oven_timer_expired_cb_t expired_cb = NULL;

static void format_str(uint8_t id)
{
    int32_t rem = timers[id].remaining_s;
    if (rem < 0) rem = 0;
    int h = rem / 3600;
    int m = (rem % 3600) / 60;
    int s = rem % 60;
    snprintf(timers[id].str_buf, sizeof(timers[id].str_buf), "%02d:%02d:%02d", h, m, s);
}

static void tick_cb(TimerHandle_t timer)
{
    for (uint8_t i = 0; i < OVEN_TIMER_COUNT; i++) {
        if (!timers[i].running) continue;
        timers[i].remaining_s--;
        format_str(i);
        if (timers[i].remaining_s <= 0) {
            timers[i].remaining_s = 0;
            timers[i].running = false;
            ESP_LOGI(TAG, "Timer %d expired", i + 1);
            if (expired_cb) {
                expired_cb(i);
            }
        }
    }
}

esp_err_t oven_timer_init(oven_timer_expired_cb_t on_expired)
{
    expired_cb = on_expired;
    memset(timers, 0, sizeof(timers));
    for (uint8_t i = 0; i < OVEN_TIMER_COUNT; i++) {
        format_str(i);
    }

    tick_timer = xTimerCreate("otick", pdMS_TO_TICKS(1000), pdTRUE, NULL, tick_cb);
    if (tick_timer) {
        xTimerStart(tick_timer, 0);
    }

    ESP_LOGI(TAG, "Oven timers ready (%d timers)", OVEN_TIMER_COUNT);
    return ESP_OK;
}

void oven_timer_set_hours(uint8_t id, int32_t hours)
{
    if (id >= OVEN_TIMER_COUNT) return;
    if (hours < 0) hours = 0;
    if (hours > 99) hours = 99;
    timers[id].preset_h = hours;
}

void oven_timer_set_minutes(uint8_t id, int32_t minutes)
{
    if (id >= OVEN_TIMER_COUNT) return;
    if (minutes < 0) minutes = 0;
    if (minutes > 59) minutes = 59;
    timers[id].preset_m = minutes;
}

void oven_timer_set_seconds(uint8_t id, int32_t seconds)
{
    if (id >= OVEN_TIMER_COUNT) return;
    if (seconds < 0) seconds = 0;
    if (seconds > 59) seconds = 59;
    timers[id].preset_s = seconds;
}

int32_t oven_timer_get_hours(uint8_t id)
{
    if (id >= OVEN_TIMER_COUNT) return 0;
    if (timers[id].running) return timers[id].remaining_s / 3600;
    return timers[id].preset_h;
}

int32_t oven_timer_get_minutes(uint8_t id)
{
    if (id >= OVEN_TIMER_COUNT) return 0;
    if (timers[id].running) return (timers[id].remaining_s % 3600) / 60;
    return timers[id].preset_m;
}

int32_t oven_timer_get_seconds(uint8_t id)
{
    if (id >= OVEN_TIMER_COUNT) return 0;
    if (timers[id].running) return timers[id].remaining_s % 60;
    return timers[id].preset_s;
}

const char *oven_timer_get_str(uint8_t id)
{
    if (id >= OVEN_TIMER_COUNT) return "00:00:00";
    if (!timers[id].running) format_str(id);
    return timers[id].str_buf;
}

void oven_timer_start(uint8_t id)
{
    if (id >= OVEN_TIMER_COUNT) return;
    int32_t total = timers[id].preset_h * 3600
                  + timers[id].preset_m * 60
                  + timers[id].preset_s;
    if (total <= 0) {
        ESP_LOGW(TAG, "Timer %d: nothing to count down (0s)", id + 1);
        return;
    }
    timers[id].remaining_s = total;
    timers[id].running = true;
    format_str(id);
    ESP_LOGI(TAG, "Timer %d started: %02d:%02d:%02d",
             id + 1, (int)timers[id].preset_h,
             (int)timers[id].preset_m, (int)timers[id].preset_s);
}

void oven_timer_stop(uint8_t id)
{
    if (id >= OVEN_TIMER_COUNT) return;
    timers[id].running = false;
    timers[id].remaining_s = 0;
    format_str(id);
    ESP_LOGI(TAG, "Timer %d stopped", id + 1);
}

bool oven_timer_is_running(uint8_t id)
{
    if (id >= OVEN_TIMER_COUNT) return false;
    return timers[id].running;
}

int32_t oven_timer_get_remaining(uint8_t id)
{
    if (id >= OVEN_TIMER_COUNT) return 0;
    return timers[id].remaining_s;
}
