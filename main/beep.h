#ifndef BEEP_H
#define BEEP_H

#include <stdint.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

#define BEEP_SOUND_1    0
#define BEEP_SOUND_2    1
#define BEEP_SOUND_3    2

esp_err_t beep_init(void);
void beep_play(uint8_t sound_id);
void beep_set_volume(uint8_t level);
uint8_t beep_get_volume(void);

#ifdef __cplusplus
}
#endif

#endif
