#ifndef RTC_CLOCK_H
#define RTC_CLOCK_H

#include <stdint.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

#define RTC_FORMAT_12H  0
#define RTC_FORMAT_24H  1

esp_err_t rtc_clock_init(void);

int32_t rtc_clock_get_hours(void);
int32_t rtc_clock_get_minutes(void);
int32_t rtc_clock_get_seconds(void);
const char *rtc_clock_get_time_str(void);

void rtc_clock_set_time(uint8_t hours, uint8_t minutes, uint8_t seconds);

void rtc_clock_set_format(uint8_t format);
uint8_t rtc_clock_get_format(void);

#ifdef __cplusplus
}
#endif

#endif
