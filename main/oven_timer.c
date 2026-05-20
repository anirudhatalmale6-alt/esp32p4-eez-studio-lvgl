#include "oven_timer.h"
#include "rtc_clock.h"
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
#include "esp_log.h"

static const char *TAG = "timer";

// --- Countdown timers ---
typedef struct {
    int32_t preset_h;
    int32_t preset_m;
    int32_t preset_s;
    int32_t remaining_s;
    bool running;
    char str_buf[12];
} timer_state_t;

static timer_state_t timers[OVEN_TIMER_COUNT];

// --- Delayed start ---
static int32_t delayed_h = 0;
static int32_t delayed_m = 0;
static bool delayed_enabled = false;
static bool delayed_fired = false;

// --- Cooking time ---
typedef struct {
    int32_t preset_h;
    int32_t preset_m;
    int32_t remaining_s;
    bool running;
    char str_buf[12];
} cooking_state_t;

static cooking_state_t cooking;

static TimerHandle_t tick_timer = NULL;
static oven_event_cb_t event_cb = NULL;

static void format_hms(char *buf, size_t sz, int32_t total_s)
{
    if (total_s < 0) total_s = 0;
    int h = total_s / 3600;
    int m = (total_s % 3600) / 60;
    int s = total_s % 60;
    snprintf(buf, sz, "%02d:%02d:%02d", h, m, s);
}

static void format_hm(char *buf, size_t sz, int32_t total_s)
{
    if (total_s < 0) total_s = 0;
    int h = total_s / 3600;
    int m = (total_s % 3600) / 60;
    snprintf(buf, sz, "%02d:%02d", h, m);
}

static void tick_cb(TimerHandle_t timer)
{
    // --- Countdown timers ---
    for (uint8_t i = 0; i < OVEN_TIMER_COUNT; i++) {
        if (!timers[i].running) continue;
        timers[i].remaining_s--;
        format_hms(timers[i].str_buf, sizeof(timers[i].str_buf), timers[i].remaining_s);
        if (timers[i].remaining_s <= 0) {
            timers[i].remaining_s = 0;
            timers[i].running = false;
            ESP_LOGI(TAG, "Timer %d expired", i + 1);
            if (event_cb) {
                event_cb(i == 0 ? OVEN_EVENT_TIMER1_EXPIRED : OVEN_EVENT_TIMER2_EXPIRED);
            }
        }
    }

    // --- Delayed start check (compare RTC time) ---
    if (delayed_enabled && !delayed_fired) {
        int32_t now_h = rtc_clock_get_hours();
        int32_t now_m = rtc_clock_get_minutes();
        if (now_h == delayed_h && now_m == delayed_m) {
            delayed_fired = true;
            delayed_enabled = false;
            ESP_LOGI(TAG, "Delayed start triggered at %02d:%02d", (int)delayed_h, (int)delayed_m);
            if (event_cb) {
                event_cb(OVEN_EVENT_DELAYED_START);
            }
        }
    }

    // --- Cooking time countdown ---
    if (cooking.running) {
        cooking.remaining_s--;
        format_hm(cooking.str_buf, sizeof(cooking.str_buf), cooking.remaining_s);
        if (cooking.remaining_s <= 0) {
            cooking.remaining_s = 0;
            cooking.running = false;
            ESP_LOGI(TAG, "Cooking time done");
            if (event_cb) {
                event_cb(OVEN_EVENT_COOKING_DONE);
            }
        }
    }
}

esp_err_t oven_timer_init(oven_event_cb_t on_event)
{
    event_cb = on_event;
    memset(timers, 0, sizeof(timers));
    memset(&cooking, 0, sizeof(cooking));

    for (uint8_t i = 0; i < OVEN_TIMER_COUNT; i++) {
        format_hms(timers[i].str_buf, sizeof(timers[i].str_buf), 0);
    }
    format_hm(cooking.str_buf, sizeof(cooking.str_buf), 0);

    tick_timer = xTimerCreate("otick", pdMS_TO_TICKS(1000), pdTRUE, NULL, tick_cb);
    if (tick_timer) {
        xTimerStart(tick_timer, 0);
    }

    ESP_LOGI(TAG, "Oven timers ready (%d alarm timers + delayed start + cooking time)", OVEN_TIMER_COUNT);
    return ESP_OK;
}

// ===================== COUNTDOWN TIMERS =====================

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
    if (!timers[id].running) {
        format_hms(timers[id].str_buf, sizeof(timers[id].str_buf),
                   timers[id].preset_h * 3600 + timers[id].preset_m * 60 + timers[id].preset_s);
    }
    return timers[id].str_buf;
}

void oven_timer_start(uint8_t id)
{
    if (id >= OVEN_TIMER_COUNT) return;
    int32_t total = timers[id].preset_h * 3600 + timers[id].preset_m * 60 + timers[id].preset_s;
    if (total <= 0) return;
    timers[id].remaining_s = total;
    timers[id].running = true;
    format_hms(timers[id].str_buf, sizeof(timers[id].str_buf), total);
    ESP_LOGI(TAG, "Timer %d started: %02d:%02d:%02d", id + 1,
             (int)timers[id].preset_h, (int)timers[id].preset_m, (int)timers[id].preset_s);
}

void oven_timer_stop(uint8_t id)
{
    if (id >= OVEN_TIMER_COUNT) return;
    timers[id].running = false;
    timers[id].remaining_s = 0;
    format_hms(timers[id].str_buf, sizeof(timers[id].str_buf), 0);
    ESP_LOGI(TAG, "Timer %d stopped", id + 1);
}

bool oven_timer_is_running(uint8_t id)
{
    if (id >= OVEN_TIMER_COUNT) return false;
    return timers[id].running;
}

// ===================== DELAYED START =====================

void oven_delayed_start_set_hours(int32_t hours)
{
    if (hours < 0) hours = 0;
    if (hours > 23) hours = 23;
    delayed_h = hours;
}

void oven_delayed_start_set_minutes(int32_t minutes)
{
    if (minutes < 0) minutes = 0;
    if (minutes > 59) minutes = 59;
    delayed_m = minutes;
}

int32_t oven_delayed_start_get_hours(void) { return delayed_h; }
int32_t oven_delayed_start_get_minutes(void) { return delayed_m; }

void oven_delayed_start_enable(void)
{
    delayed_fired = false;
    delayed_enabled = true;
    ESP_LOGI(TAG, "Delayed start armed for %02d:%02d", (int)delayed_h, (int)delayed_m);
}

void oven_delayed_start_disable(void)
{
    delayed_enabled = false;
    ESP_LOGI(TAG, "Delayed start disabled");
}

bool oven_delayed_start_is_enabled(void) { return delayed_enabled; }

// ===================== COOKING TIME =====================

void oven_cooking_set_hours(int32_t hours)
{
    if (hours < 0) hours = 0;
    if (hours > 99) hours = 99;
    cooking.preset_h = hours;
}

void oven_cooking_set_minutes(int32_t minutes)
{
    if (minutes < 0) minutes = 0;
    if (minutes > 59) minutes = 59;
    cooking.preset_m = minutes;
}

int32_t oven_cooking_get_hours(void)
{
    if (cooking.running) return cooking.remaining_s / 3600;
    return cooking.preset_h;
}

int32_t oven_cooking_get_minutes(void)
{
    if (cooking.running) return (cooking.remaining_s % 3600) / 60;
    return cooking.preset_m;
}

const char *oven_cooking_get_str(void)
{
    if (!cooking.running) {
        format_hm(cooking.str_buf, sizeof(cooking.str_buf),
                  cooking.preset_h * 3600 + cooking.preset_m * 60);
    }
    return cooking.str_buf;
}

void oven_cooking_start(void)
{
    int32_t total = cooking.preset_h * 3600 + cooking.preset_m * 60;
    if (total <= 0) return;
    cooking.remaining_s = total;
    cooking.running = true;
    format_hm(cooking.str_buf, sizeof(cooking.str_buf), total);
    ESP_LOGI(TAG, "Cooking started: %02d:%02d", (int)cooking.preset_h, (int)cooking.preset_m);
}

void oven_cooking_stop(void)
{
    cooking.running = false;
    cooking.remaining_s = 0;
    format_hm(cooking.str_buf, sizeof(cooking.str_buf), 0);
    ESP_LOGI(TAG, "Cooking stopped");
}

bool oven_cooking_is_running(void) { return cooking.running; }
