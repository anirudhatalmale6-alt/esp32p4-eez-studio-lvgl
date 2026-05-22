#ifndef PRIZER_BUS_H
#define PRIZER_BUS_H

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

// Frame structure for received (parsed) frames
typedef struct {
    uint8_t  dvnum;
    uint8_t  uart_rev;
    char     id[4];
    uint8_t  id_cfg;
    char     cmd[7];
    uint16_t data1;
    uint16_t data2;
    uint16_t crc;
    uint8_t  valid;
} prizer_frame_in_t;

// Callback receives the full parsed frame after CRC validation
typedef void (*prizer_rx_cmd_cb_t)(const prizer_frame_in_t *frame);

esp_err_t prizer_bus_init(prizer_rx_cmd_cb_t on_rx_cmd);

// UIA command variables (set from UI, transmitted over UART)
void prizer_set_setpoint_temp_oven1(uint16_t temp);
void prizer_set_operating_mode_oven1(uint8_t mode);
void prizer_set_setpoint_temp_oven2(uint16_t temp);
void prizer_set_operating_mode_oven2(uint8_t mode);

uint16_t prizer_get_setpoint_temp_oven1(void);
uint8_t  prizer_get_operating_mode_oven1(void);
uint16_t prizer_get_setpoint_temp_oven2(void);
uint8_t  prizer_get_operating_mode_oven2(void);

// Bus status
bool prizer_is_active(void);

#ifdef __cplusplus
}
#endif

#endif
