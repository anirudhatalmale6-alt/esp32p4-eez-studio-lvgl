// Prizer bus communication — ESP32-P4 port (UART IN only)
// Ported from XMC1402 prizer_bus_interface.c
// Protocol: 32-byte ASCII frames, comma-separated, CRC-8 (poly 0x07)
// UIA device: sends OVSP@1, OVMD@1, OVSP@2, OVMD@2 commands

#include "prizer_bus.h"
#include <string.h>
#include <stdio.h>
#include "driver/uart.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "freertos/semphr.h"
#include "esp_log.h"

static const char *TAG = "prizer";

// --- UART configuration ---
#define PRIZER_UART_NUM       UART_NUM_1
#define PRIZER_TX_PIN         GPIO_NUM_10
#define PRIZER_RX_PIN         GPIO_NUM_11
#define PRIZER_BAUD           256000
#define PRIZER_FRAME_SIZE     32
#define PRIZER_RX_BUF_SIZE    256
#define PRIZER_TX_BUF_SIZE    256

// --- Protocol constants ---
#define ID_PPSW_REV           1
#define ID_VARIANT            "UIA"
#define ID_CFG_DEFAULT        1
#define NBE_TICKS_INACTIVITY  50

// --- UIA commands ---
static const char cmd_device[10][7] = {
    "OVSP@1", "OVMD@1", "OVSP@2", "OVMD@2",
    "", "", "", "", "", ""
};
#define CMD_COUNT 10

// --- UIA external variables ---
static uint16_t setpoint_temperature_oven_1 = 0;
static uint8_t  operating_mode_oven_1 = 0;
static uint16_t setpoint_temperature_oven_2 = 0;
static uint8_t  operating_mode_oven_2 = 0;

// --- Bus state ---
static uint8_t  uart_tx_data[PRIZER_FRAME_SIZE];
static uint16_t device_num = 1;
static uint8_t  first_device = 1;
static uint16_t counter_inactivity = 0;
static uint8_t  flag_active = 0;
static uint8_t  frame_index_tx = 0;

static prizer_rx_cmd_cb_t rx_cmd_cb = NULL;
static TaskHandle_t tx_task_handle = NULL;
static SemaphoreHandle_t tx_mutex = NULL;

// ========================= CRC-8 =========================

static const uint8_t crc8_table[256] = {
    0x00,0x07,0x0E,0x09,0x1C,0x1B,0x12,0x15,0x38,0x3F,0x36,0x31,0x24,0x23,0x2A,0x2D,
    0x70,0x77,0x7E,0x79,0x6C,0x6B,0x62,0x65,0x48,0x4F,0x46,0x41,0x54,0x53,0x5A,0x5D,
    0xE0,0xE7,0xEE,0xE9,0xFC,0xFB,0xF2,0xF5,0xD8,0xDF,0xD6,0xD1,0xC4,0xC3,0xCA,0xCD,
    0x90,0x97,0x9E,0x99,0x8C,0x8B,0x82,0x85,0xA8,0xAF,0xA6,0xA1,0xB4,0xB3,0xBA,0xBD,
    0xC7,0xC0,0xC9,0xCE,0xDB,0xDC,0xD5,0xD2,0xFF,0xF8,0xF1,0xF6,0xE3,0xE4,0xED,0xEA,
    0xB7,0xB0,0xB9,0xBE,0xAB,0xAC,0xA5,0xA2,0x8F,0x88,0x81,0x86,0x93,0x94,0x9D,0x9A,
    0x27,0x20,0x29,0x2E,0x3B,0x3C,0x35,0x32,0x1F,0x18,0x11,0x16,0x03,0x04,0x0D,0x0A,
    0x57,0x50,0x59,0x5E,0x4B,0x4C,0x45,0x42,0x6F,0x68,0x61,0x66,0x73,0x74,0x7D,0x7A,
    0x89,0x8E,0x87,0x80,0x95,0x92,0x9B,0x9C,0xB1,0xB6,0xBF,0xB8,0xAD,0xAA,0xA3,0xA4,
    0xF9,0xFE,0xF7,0xF0,0xE5,0xE2,0xEB,0xEC,0xC1,0xC6,0xCF,0xC8,0xDD,0xDA,0xD3,0xD4,
    0x69,0x6E,0x67,0x60,0x75,0x72,0x7B,0x7C,0x51,0x56,0x5F,0x58,0x4D,0x4A,0x43,0x44,
    0x19,0x1E,0x17,0x10,0x05,0x02,0x0B,0x0C,0x21,0x26,0x2F,0x28,0x3D,0x3A,0x33,0x34,
    0x4E,0x49,0x40,0x47,0x52,0x55,0x5C,0x5B,0x76,0x71,0x78,0x7F,0x6A,0x6D,0x64,0x63,
    0x3E,0x39,0x30,0x37,0x22,0x25,0x2C,0x2B,0x06,0x01,0x08,0x0F,0x1A,0x1D,0x14,0x13,
    0xAE,0xA9,0xA0,0xA7,0xB2,0xB5,0xBC,0xBB,0x96,0x91,0x98,0x9F,0x8A,0x8D,0x84,0x83,
    0xDE,0xD9,0xD0,0xD7,0xC2,0xC5,0xCC,0xCB,0xE6,0xE1,0xE8,0xEF,0xFA,0xFD,0xF4,0xF3
};

static uint8_t crc8_calc(const uint8_t *data, uint8_t len)
{
    uint8_t crc = 0xFF;
    for (uint8_t i = 0; i < len; i++) {
        crc = crc8_table[crc ^ data[i]];
    }
    return crc;
}

// ========================= ASCII HELPERS =========================

static uint8_t uint8_to_str(uint8_t value, uint8_t *buf, uint8_t idx)
{
    if (value == 0) {
        buf[idx++] = '0';
        return idx;
    }
    uint8_t div = 100;
    uint8_t started = 0;
    while (div > 0) {
        uint8_t digit = value / div;
        if (digit != 0 || started || div == 1) {
            buf[idx++] = '0' + digit;
            started = 1;
        }
        value %= div;
        div /= 10;
    }
    return idx;
}

static uint8_t uint16_to_str(uint16_t value, uint8_t *buf, uint8_t idx)
{
    if (value == 0) {
        buf[idx++] = '0';
        return idx;
    }
    uint16_t div = 10000;
    uint8_t started = 0;
    while (div > 0) {
        uint8_t digit = value / div;
        if (digit != 0 || started || div == 1) {
            buf[idx++] = '0' + digit;
            started = 1;
        }
        value %= div;
        div /= 10;
    }
    return idx;
}

// ========================= FRAME BUILD =========================

static uint8_t build_frame(uint8_t dvnum, uint8_t uart_rev, const char *id,
                           uint8_t id_cfg, const char *cmd, uint16_t data1, uint16_t data2)
{
    uint8_t len = 0;
    memset(uart_tx_data, 0, PRIZER_FRAME_SIZE);

    len = uint8_to_str(dvnum, uart_tx_data, len);
    uart_tx_data[len++] = ',';

    len = uint8_to_str(uart_rev, uart_tx_data, len);
    uart_tx_data[len++] = ',';

    for (uint8_t i = 0; i < 3 && id[i]; i++) uart_tx_data[len++] = id[i];
    uart_tx_data[len++] = ',';

    len = uint8_to_str(id_cfg, uart_tx_data, len);
    uart_tx_data[len++] = ',';

    for (uint8_t i = 0; i < 6 && cmd[i]; i++) uart_tx_data[len++] = cmd[i];
    uart_tx_data[len++] = ',';

    len = uint16_to_str(data1, uart_tx_data, len);
    uart_tx_data[len++] = ',';
    len = uint16_to_str(data2, uart_tx_data, len);

    uint8_t crc = crc8_calc(uart_tx_data, len);
    uart_tx_data[len++] = ',';
    len = uint8_to_str(crc, uart_tx_data, len);

    return len;
}

// ========================= FRAME PARSE =========================

static prizer_frame_in_t parse_frame(const uint8_t *rx)
{
    prizer_frame_in_t f = {0};

    uint8_t crc_start = 0;
    for (uint8_t i = 0; i < PRIZER_FRAME_SIZE && rx[i]; i++) {
        if (rx[i] == ',') crc_start = i;
    }

    uint16_t received_crc = 0;
    for (uint8_t i = crc_start + 1; i < PRIZER_FRAME_SIZE && rx[i]; i++) {
        if (rx[i] < '0' || rx[i] > '9') return f;
        received_crc = received_crc * 10 + (rx[i] - '0');
    }

    uint16_t computed_crc = crc8_calc(rx, crc_start);
    if (computed_crc != received_crc) return f;

    f.crc = received_crc;
    f.valid = 1;

    uint8_t field = 0;
    uint8_t start = 0;

    for (uint8_t pos = 0; pos <= crc_start; pos++) {
        if (rx[pos] == ',' || pos == crc_start) {
            uint8_t len = 0;
            for (uint8_t i = start; i <= pos && i <= crc_start; i++) {
                if (rx[i] == ',' || rx[i] == '\0') break;
                len++;
            }

            switch (field) {
                case 0:
                    if (len == 1) f.dvnum = rx[start] - '0';
                    else if (len == 2) f.dvnum = (rx[start]-'0')*10 + (rx[start+1]-'0');
                    else if (len == 3) f.dvnum = (rx[start]-'0')*100 + (rx[start+1]-'0')*10 + (rx[start+2]-'0');
                    break;
                case 1:
                    if (len == 1) f.uart_rev = rx[start] - '0';
                    else if (len == 2) f.uart_rev = (rx[start]-'0')*10 + (rx[start+1]-'0');
                    break;
                case 2:
                    if (len <= 3) { memcpy(f.id, &rx[start], len); f.id[len] = '\0'; }
                    break;
                case 3:
                    if (len == 1) f.id_cfg = rx[start] - '0';
                    else if (len == 2) f.id_cfg = (rx[start]-'0')*10 + (rx[start+1]-'0');
                    break;
                case 4:
                    if (len <= 6) { memcpy(f.cmd, &rx[start], len); f.cmd[len] = '\0'; }
                    break;
                case 5: {
                    uint32_t v = 0;
                    for (uint8_t i = 0; i < len; i++) v = v * 10 + (rx[start+i] - '0');
                    f.data1 = (uint16_t)v;
                    break;
                }
                case 6: {
                    uint32_t v = 0;
                    for (uint8_t i = 0; i < len; i++) v = v * 10 + (rx[start+i] - '0');
                    f.data2 = (uint16_t)v;
                    break;
                }
                default: break;
            }
            field++;
            start = pos + 1;
        }
    }
    return f;
}

// ========================= UIA COMMAND DATA =========================

static uint16_t get_command_data(const char *cmd, int field)
{
    if (field == 1) {
        if (strcmp(cmd, "OVSP@1") == 0) return setpoint_temperature_oven_1;
        if (strcmp(cmd, "OVMD@1") == 0) return operating_mode_oven_1;
        if (strcmp(cmd, "OVSP@2") == 0) return setpoint_temperature_oven_2;
        if (strcmp(cmd, "OVMD@2") == 0) return operating_mode_oven_2;
    }
    return 0;
}

// ========================= TX (mutex-protected) =========================

static void send_frame(const uint8_t *data, uint8_t len)
{
    uart_write_bytes(PRIZER_UART_NUM, data, PRIZER_FRAME_SIZE);
}

static void send_device_frames(void)
{
    for (uint8_t i = 0; i < CMD_COUNT; i++) {
        if (cmd_device[i][0] == '\0') break;

        uint16_t d1 = get_command_data(cmd_device[i], 1);
        uint16_t d2 = get_command_data(cmd_device[i], 2);

        build_frame(device_num, ID_PPSW_REV, ID_VARIANT, ID_CFG_DEFAULT,
                    cmd_device[i], d1, d2);
        send_frame(uart_tx_data, PRIZER_FRAME_SIZE);
    }

    build_frame(device_num, ID_PPSW_REV, ID_VARIANT, ID_CFG_DEFAULT,
                "END@FR", 0, 0);
    send_frame(uart_tx_data, PRIZER_FRAME_SIZE);
}

// ========================= TX TASK =========================
// Handles inactivity-triggered frame sends without blocking the timer daemon

static void prizer_tx_task(void *arg)
{
    while (1) {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        xSemaphoreTake(tx_mutex, portMAX_DELAY);
        uart_flush_input(PRIZER_UART_NUM);
        frame_index_tx = 0;
        send_device_frames();
        xSemaphoreGive(tx_mutex);
    }
}

// ========================= ACTIVITY CHECK (20ms) =========================
// Runs in FreeRTOS timer daemon context — must NEVER block

static void activity_timer_cb(TimerHandle_t timer)
{
    uint16_t limit = first_device ? NBE_TICKS_INACTIVITY : (NBE_TICKS_INACTIVITY * 3);

    if (counter_inactivity < limit) {
        counter_inactivity++;
    } else {
        counter_inactivity = 0;
        flag_active = 0;

        if (first_device) {
            device_num = 1;
        }

        if (tx_task_handle) {
            xTaskNotifyGive(tx_task_handle);
        }
    }
}

// ========================= RX TASK =========================

static void prizer_rx_task(void *arg)
{
    uint8_t rx_buf[PRIZER_FRAME_SIZE];

    while (1) {
        int bytes_read = uart_read_bytes(PRIZER_UART_NUM, rx_buf, PRIZER_FRAME_SIZE,
                                          pdMS_TO_TICKS(100));
        if (bytes_read < PRIZER_FRAME_SIZE) continue;

        first_device = 0;

        prizer_frame_in_t frame = parse_frame(rx_buf);

        if (frame.valid && frame.id[0] != '\0' && frame.cmd[0] != '\0') {
            device_num = frame.dvnum + 1;
            counter_inactivity = 0;
            flag_active = 1;

            if (rx_cmd_cb) {
                rx_cmd_cb(&frame);
            }

            xSemaphoreTake(tx_mutex, portMAX_DELAY);
            if (strcmp(frame.cmd, "END@FR") != 0) {
                send_frame(rx_buf, PRIZER_FRAME_SIZE);
            } else {
                send_device_frames();
            }
            xSemaphoreGive(tx_mutex);
        }
    }
}

// ========================= PUBLIC API =========================

esp_err_t prizer_bus_init(prizer_rx_cmd_cb_t on_rx_cmd)
{
    rx_cmd_cb = on_rx_cmd;

    tx_mutex = xSemaphoreCreateMutex();

    uart_config_t uart_config = {
        .baud_rate  = PRIZER_BAUD,
        .data_bits  = UART_DATA_8_BITS,
        .parity     = UART_PARITY_DISABLE,
        .stop_bits  = UART_STOP_BITS_1,
        .flow_ctrl  = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };

    ESP_ERROR_CHECK(uart_driver_install(PRIZER_UART_NUM, PRIZER_RX_BUF_SIZE, PRIZER_TX_BUF_SIZE, 0, NULL, 0));
    ESP_ERROR_CHECK(uart_param_config(PRIZER_UART_NUM, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(PRIZER_UART_NUM, PRIZER_TX_PIN, PRIZER_RX_PIN,
                                  UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));

    gpio_set_pull_mode(PRIZER_RX_PIN, GPIO_PULLUP_ONLY);

    TimerHandle_t act_timer = xTimerCreate("prizer_act", pdMS_TO_TICKS(20),
                                            pdTRUE, NULL, activity_timer_cb);
    if (act_timer) xTimerStart(act_timer, 0);

    xTaskCreate(prizer_tx_task, "prizer_tx", 4096, NULL, 3, &tx_task_handle);
    xTaskCreate(prizer_rx_task, "prizer_rx", 4096, NULL, 3, NULL);

    ESP_LOGI(TAG, "Prizer bus ready (UART%d TX=GPIO%d RX=GPIO%d %d baud)",
             PRIZER_UART_NUM, PRIZER_TX_PIN, PRIZER_RX_PIN, PRIZER_BAUD);
    return ESP_OK;
}

void prizer_set_setpoint_temp_oven1(uint16_t temp) { setpoint_temperature_oven_1 = temp; }
void prizer_set_operating_mode_oven1(uint8_t mode) { operating_mode_oven_1 = mode; }
void prizer_set_setpoint_temp_oven2(uint16_t temp) { setpoint_temperature_oven_2 = temp; }
void prizer_set_operating_mode_oven2(uint8_t mode) { operating_mode_oven_2 = mode; }

uint16_t prizer_get_setpoint_temp_oven1(void) { return setpoint_temperature_oven_1; }
uint8_t  prizer_get_operating_mode_oven1(void) { return operating_mode_oven_1; }
uint16_t prizer_get_setpoint_temp_oven2(void) { return setpoint_temperature_oven_2; }
uint8_t  prizer_get_operating_mode_oven2(void) { return operating_mode_oven_2; }

bool prizer_is_active(void) { return flag_active != 0; }
