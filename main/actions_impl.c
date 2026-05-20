#include "beep.h"
#include "rtc_clock.h"
#include "oven_timer.h"
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

void action_toggle_time_format(lv_event_t *e) {
    (void)e;
    uint8_t fmt = rtc_clock_get_format();
    rtc_clock_set_format(fmt == RTC_FORMAT_24H ? RTC_FORMAT_12H : RTC_FORMAT_24H);
}

// --- Countdown timers ---
void action_timer1_start(lv_event_t *e) { (void)e; oven_timer_start(0); }
void action_timer1_stop(lv_event_t *e)  { (void)e; oven_timer_stop(0); }
void action_timer2_start(lv_event_t *e) { (void)e; oven_timer_start(1); }
void action_timer2_stop(lv_event_t *e)  { (void)e; oven_timer_stop(1); }

// --- Delayed start ---
void action_delayed_start_enable(lv_event_t *e)  { (void)e; oven_delayed_start_enable(); }
void action_delayed_start_disable(lv_event_t *e) { (void)e; oven_delayed_start_disable(); }

// --- Cooking time ---
void action_cooking_start(lv_event_t *e) { (void)e; oven_cooking_start(); }
void action_cooking_stop(lv_event_t *e)  { (void)e; oven_cooking_stop(); }
