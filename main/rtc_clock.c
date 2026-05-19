#include "rtc_clock.h"
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "nvs.h"

static const char *TAG = "rtc";

#define NVS_NAMESPACE    "rtc"
#define NVS_KEY_EPOCH    "epoch"
#define NVS_SAVE_INTERVAL_S 60

static char time_str_buf[12];
static TimerHandle_t save_timer = NULL;

static void nvs_save_epoch(void)
{
    time_t now;
    time(&now);
    nvs_handle_t h;
    if (nvs_open(NVS_NAMESPACE, NVS_READWRITE, &h) == ESP_OK) {
        nvs_set_i32(h, NVS_KEY_EPOCH, (int32_t)now);
        nvs_commit(h);
        nvs_close(h);
    }
}

static time_t nvs_load_epoch(void)
{
    int32_t val = 0;
    nvs_handle_t h;
    if (nvs_open(NVS_NAMESPACE, NVS_READONLY, &h) == ESP_OK) {
        nvs_get_i32(h, NVS_KEY_EPOCH, &val);
        nvs_close(h);
    }
    return (time_t)val;
}

static void save_timer_cb(TimerHandle_t timer)
{
    nvs_save_epoch();
}

esp_err_t rtc_clock_init(void)
{
    time_t saved = nvs_load_epoch();
    if (saved > 0) {
        struct timeval tv = { .tv_sec = saved, .tv_usec = 0 };
        settimeofday(&tv, NULL);
        struct tm t;
        localtime_r(&saved, &t);
        ESP_LOGI(TAG, "Restored time: %02d:%02d:%02d", t.tm_hour, t.tm_min, t.tm_sec);
    } else {
        ESP_LOGI(TAG, "No saved time, starting at 00:00:00");
    }

    save_timer = xTimerCreate("rtc_save", pdMS_TO_TICKS(NVS_SAVE_INTERVAL_S * 1000),
                               pdTRUE, NULL, save_timer_cb);
    if (save_timer) {
        xTimerStart(save_timer, 0);
    }

    ESP_LOGI(TAG, "RTC clock ready (32.768 kHz XTAL, NVS backup every %ds)", NVS_SAVE_INTERVAL_S);
    return ESP_OK;
}

static void get_localtime(struct tm *t)
{
    time_t now;
    time(&now);
    localtime_r(&now, t);
}

int32_t rtc_clock_get_hours(void)
{
    struct tm t;
    get_localtime(&t);
    return t.tm_hour;
}

int32_t rtc_clock_get_minutes(void)
{
    struct tm t;
    get_localtime(&t);
    return t.tm_min;
}

int32_t rtc_clock_get_seconds(void)
{
    struct tm t;
    get_localtime(&t);
    return t.tm_sec;
}

const char *rtc_clock_get_time_str(void)
{
    struct tm t;
    get_localtime(&t);
    int hour12 = t.tm_hour % 12;
    if (hour12 == 0) hour12 = 12;
    const char *ampm = (t.tm_hour < 12) ? "AM" : "PM";
    snprintf(time_str_buf, sizeof(time_str_buf), "%d:%02d %s", hour12, t.tm_min, ampm);
    return time_str_buf;
}

void rtc_clock_set_time(uint8_t hours, uint8_t minutes, uint8_t seconds)
{
    if (hours > 23) hours = 23;
    if (minutes > 59) minutes = 59;
    if (seconds > 59) seconds = 59;

    time_t now;
    time(&now);
    struct tm t;
    localtime_r(&now, &t);

    t.tm_hour = hours;
    t.tm_min = minutes;
    t.tm_sec = seconds;

    time_t new_time = mktime(&t);
    struct timeval tv = { .tv_sec = new_time, .tv_usec = 0 };
    settimeofday(&tv, NULL);

    nvs_save_epoch();

    ESP_LOGI(TAG, "Time set to %02d:%02d:%02d", hours, minutes, seconds);
}
