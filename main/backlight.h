#ifndef BACKLIGHT_H
#define BACKLIGHT_H

#include <stdint.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t backlight_init(void);
void backlight_set_brightness(uint8_t percent);
uint8_t backlight_get_brightness(void);

#ifdef __cplusplus
}
#endif

#endif
