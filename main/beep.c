#include "beep.h"
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/i2s_std.h"
#include "esp_log.h"

static const char *TAG = "beep";

#define I2S_LRC_GPIO    GPIO_NUM_15
#define I2S_BCLK_GPIO   GPIO_NUM_14
#define I2S_DIN_GPIO    GPIO_NUM_13

extern const uint8_t beep1_wav_start[] asm("_binary_beep1_wav_start");
extern const uint8_t beep1_wav_end[]   asm("_binary_beep1_wav_end");
extern const uint8_t beep2_wav_start[] asm("_binary_beep2_wav_start");
extern const uint8_t beep2_wav_end[]   asm("_binary_beep2_wav_end");

typedef struct {
    uint32_t sample_rate;
    uint16_t num_channels;
    uint16_t bits_per_sample;
    const uint8_t *data;
    uint32_t data_size;
} wav_info_t;

static i2s_chan_handle_t tx_handle = NULL;
static QueueHandle_t beep_queue = NULL;
static volatile uint8_t volume_level = 80;
static wav_info_t wav_sounds[2];
static bool i2s_running = false;

static esp_err_t wav_parse(const uint8_t *buf, size_t buf_size, wav_info_t *info)
{
    if (buf_size < 44 || memcmp(buf, "RIFF", 4) != 0 || memcmp(buf + 8, "WAVE", 4) != 0)
        return ESP_ERR_INVALID_ARG;

    const uint8_t *ptr = buf + 12;
    const uint8_t *end = buf + buf_size;
    bool fmt_found = false;

    while (ptr + 8 <= end) {
        uint32_t chunk_size = *(uint32_t *)(ptr + 4);

        if (memcmp(ptr, "fmt ", 4) == 0 && ptr + 8 + chunk_size <= end) {
            info->num_channels    = *(uint16_t *)(ptr + 10);
            info->sample_rate     = *(uint32_t *)(ptr + 12);
            info->bits_per_sample = *(uint16_t *)(ptr + 22);
            fmt_found = true;
        }

        if (memcmp(ptr, "data", 4) == 0 && fmt_found) {
            info->data_size = chunk_size;
            info->data = ptr + 8;
            if (info->data + info->data_size > end)
                info->data_size = end - info->data;
            return ESP_OK;
        }

        ptr += 8 + chunk_size;
        if (chunk_size & 1) ptr++;
    }

    return ESP_ERR_NOT_FOUND;
}

static esp_err_t i2s_init(uint32_t sample_rate)
{
    i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_AUTO, I2S_ROLE_MASTER);
    chan_cfg.auto_clear_after_cb = true;
    chan_cfg.auto_clear_before_cb = true;
    ESP_RETURN_ON_ERROR(i2s_new_channel(&chan_cfg, &tx_handle, NULL), TAG, "new channel");

    i2s_std_config_t std_cfg = {
        .clk_cfg  = I2S_STD_CLK_DEFAULT_CONFIG(sample_rate),
        .slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_MONO),
        .gpio_cfg = {
            .mclk = I2S_GPIO_UNUSED,
            .bclk = I2S_BCLK_GPIO,
            .ws   = I2S_LRC_GPIO,
            .dout = I2S_DIN_GPIO,
            .din  = I2S_GPIO_UNUSED,
            .invert_flags = { .mclk_inv = false, .bclk_inv = false, .ws_inv = false },
        },
    };
    ESP_RETURN_ON_ERROR(i2s_channel_init_std_mode(tx_handle, &std_cfg), TAG, "init std");
    ESP_LOGI(TAG, "I2S ready: %lu Hz, 16-bit mono", (unsigned long)sample_rate);
    return ESP_OK;
}

static void play_wav(const wav_info_t *wav)
{
    if (!i2s_running) {
        i2s_channel_enable(tx_handle);
        i2s_running = true;
    }

    const size_t chunk_size = 1024;
    uint8_t scaled_buf[1024];
    size_t remaining = wav->data_size;
    const uint8_t *ptr = wav->data;
    uint8_t vol = volume_level;
    uint32_t scale = (uint32_t)vol * 256 / 100;

    while (remaining > 0) {
        size_t to_write = (remaining < chunk_size) ? remaining : chunk_size;
        size_t bytes_written = 0;
        const uint8_t *write_ptr;

        if (vol == 0) {
            memset(scaled_buf, 0, to_write);
            write_ptr = scaled_buf;
        } else if (vol < 100 && wav->bits_per_sample == 16) {
            const int16_t *in  = (const int16_t *)ptr;
            int16_t       *out = (int16_t *)scaled_buf;
            size_t n = to_write / 2;
            for (size_t i = 0; i < n; i++)
                out[i] = (int16_t)(((int32_t)in[i] * scale) >> 8);
            write_ptr = scaled_buf;
        } else {
            write_ptr = ptr;
        }

        i2s_channel_write(tx_handle, write_ptr, to_write, &bytes_written, pdMS_TO_TICKS(500));
        ptr += bytes_written;
        remaining -= bytes_written;
    }

    // Flush with silence to avoid residual noise
    memset(scaled_buf, 0, chunk_size);
    size_t written;
    for (int i = 0; i < 4; i++)
        i2s_channel_write(tx_handle, scaled_buf, chunk_size, &written, pdMS_TO_TICKS(100));

    i2s_channel_disable(tx_handle);
    i2s_running = false;
}

static void beep_task(void *arg)
{
    uint8_t sound_id;
    while (1) {
        if (xQueueReceive(beep_queue, &sound_id, portMAX_DELAY) == pdTRUE) {
            // Drain any queued duplicates so we don't repeat
            while (xQueueReceive(beep_queue, &sound_id, 0) == pdTRUE) {}

            if (sound_id < 2 && wav_sounds[sound_id].data != NULL) {
                play_wav(&wav_sounds[sound_id]);
            }
        }
    }
}

esp_err_t beep_init(void)
{
    esp_err_t ret;

    ret = wav_parse(beep1_wav_start, beep1_wav_end - beep1_wav_start, &wav_sounds[0]);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to parse beep1.wav");
        return ret;
    }
    ESP_LOGI(TAG, "beep1: %lu Hz, %d-bit, %lu bytes",
             (unsigned long)wav_sounds[0].sample_rate, wav_sounds[0].bits_per_sample,
             (unsigned long)wav_sounds[0].data_size);

    ret = wav_parse(beep2_wav_start, beep2_wav_end - beep2_wav_start, &wav_sounds[1]);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to parse beep2.wav");
        return ret;
    }
    ESP_LOGI(TAG, "beep2: %lu Hz, %d-bit, %lu bytes",
             (unsigned long)wav_sounds[1].sample_rate, wav_sounds[1].bits_per_sample,
             (unsigned long)wav_sounds[1].data_size);

    ret = i2s_init(wav_sounds[0].sample_rate);
    if (ret != ESP_OK) return ret;

    beep_queue = xQueueCreate(4, sizeof(uint8_t));
    xTaskCreate(beep_task, "beep", 4096, NULL, 3, NULL);

    ESP_LOGI(TAG, "Beep module ready (2 sounds, volume %d%%)", volume_level);
    return ESP_OK;
}

void beep_play(uint8_t sound_id)
{
    if (beep_queue && sound_id < 2) {
        xQueueSend(beep_queue, &sound_id, 0);
    }
}

void beep_set_volume(uint8_t level)
{
    if (level > 100) level = 100;
    volume_level = level;
}

uint8_t beep_get_volume(void)
{
    return volume_level;
}
