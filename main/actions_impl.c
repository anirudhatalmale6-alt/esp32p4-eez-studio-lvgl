// ============================================================================
// EEZ Studio User Action Implementations
// ============================================================================
// Each user action defined in EEZ Studio generates a declaration in actions.h.
// This file provides the implementations.
//
// User Actions to create in EEZ Studio:
//   beep1      - play beep sound 1
//   beep2      - play beep sound 2
//   set_time   - apply rtc_hours + rtc_minutes to the RTC clock
// ============================================================================

#include "beep.h"
#include "rtc_clock.h"
#include "lvgl.h"

void action_beep1(lv_event_t *e) {
    (void)e;
    beep_play(BEEP_SOUND_1);
}

void action_beep2(lv_event_t *e) {
    (void)e;
    beep_play(BEEP_SOUND_2);
}

void action_set_time(lv_event_t *e) {
    (void)e;
    rtc_clock_set_time(
        (uint8_t)rtc_clock_get_hours(),
        (uint8_t)rtc_clock_get_minutes(),
        0
    );
}
