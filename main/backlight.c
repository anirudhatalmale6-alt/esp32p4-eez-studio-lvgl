#include "backlight.h"
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
#include "driver/ledc.h"
#include "driver/gpio.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_log.h"

static const char *TAG = "backlight";

#define BL_GPIO          GPIO_NUM_23
#define BL_LEDC_TIMER    LEDC_TIMER_0
#define BL_LEDC_MODE     LEDC_LOW_SPEED_MODE
#define BL_LEDC_CHANNEL  LEDC_CHANNEL_0
#define BL_LEDC_FREQ_HZ  5000
#define BL_LEDC_RES      LEDC_TIMER_10_BIT

#define NVS_NAMESPACE    "backlight"
#define NVS_KEY_BRIGHT   "brightness"
#define NVS_SAVE_DELAY_MS 2000

static volatile uint8_t brightness_pct = 80;
static TimerHandle_t nvs_save_timer = NULL;
static volatile uint8_t nvs_pending_brightness = 0;

static void nvs_save_timer_cb(TimerHandle_t timer)
{
    uint8_t val = nvs_pending_brightness;
    nvs_handle_t h;
    if (nvs_open(NVS_NAMESPACE, NVS_READWRITE, &h) == ESP_OK) {
        nvs_set_u8(h, NVS_KEY_BRIGHT, val);
        nvs_commit(h);
        nvs_close(h);
        ESP_LOGI(TAG, "Brightness %d%% saved to flash", val);
    }
}

static void nvs_save_deferred(uint8_t val)
{
    nvs_pending_brightness = val;
    if (nvs_save_timer) {
        xTimerReset(nvs_save_timer, 0);
    }
}

static uint8_t nvs_load_brightness(uint8_t default_val)
{
    nvs_handle_t h;
    uint8_t val = default_val;
    if (nvs_open(NVS_NAMESPACE, NVS_READONLY, &h) == ESP_OK) {
        nvs_get_u8(h, NVS_KEY_BRIGHT, &val);
        nvs_close(h);
    }
    return val;
}

static void apply_duty(uint8_t pct)
{
    uint32_t duty = 1023 - (uint32_t)pct * 1023 / 100;
    ledc_set_duty(BL_LEDC_MODE, BL_LEDC_CHANNEL, duty);
    ledc_update_duty(BL_LEDC_MODE, BL_LEDC_CHANNEL);
}

esp_err_t backlight_init(void)
{
    ledc_timer_config_t timer_cfg = {
        .speed_mode = BL_LEDC_MODE,
        .timer_num = BL_LEDC_TIMER,
        .duty_resolution = BL_LEDC_RES,
        .freq_hz = BL_LEDC_FREQ_HZ,
        .clk_cfg = LEDC_AUTO_CLK,
    };
    ESP_ERROR_CHECK(ledc_timer_config(&timer_cfg));

    brightness_pct = nvs_load_brightness(80);

    ledc_channel_config_t chan_cfg = {
        .gpio_num = BL_GPIO,
        .speed_mode = BL_LEDC_MODE,
        .channel = BL_LEDC_CHANNEL,
        .timer_sel = BL_LEDC_TIMER,
        .duty = 1023 - (uint32_t)brightness_pct * 1023 / 100,
        .hpoint = 0,
    };
    ESP_ERROR_CHECK(ledc_channel_config(&chan_cfg));

    nvs_save_timer = xTimerCreate("nvs_bl", pdMS_TO_TICKS(NVS_SAVE_DELAY_MS),
                                   pdFALSE, NULL, nvs_save_timer_cb);

    ESP_LOGI(TAG, "Backlight ready on GPIO %d, brightness %d%%", BL_GPIO, brightness_pct);
    return ESP_OK;
}

void backlight_set_brightness(uint8_t percent)
{
    if (percent > 100) percent = 100;
    brightness_pct = percent;
    apply_duty(percent);
    nvs_save_deferred(percent);
}

uint8_t backlight_get_brightness(void)
{
    return brightness_pct;
}
