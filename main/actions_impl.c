// ============================================================================
// EEZ Studio User Action Implementations
// ============================================================================
// In EEZ Studio, add User Actions named "beep1" and "beep2".
// EEZ will generate declarations in actions.h like:
//   extern void action_beep1(lv_event_t *e);
//   extern void action_beep2(lv_event_t *e);
// This file provides the implementations.
// ============================================================================

#include "beep.h"
#include "lvgl.h"

void action_beep1(lv_event_t *e) {
    (void)e;
    beep_play(BEEP_SOUND_1);
}

void action_beep2(lv_event_t *e) {
    (void)e;
    beep_play(BEEP_SOUND_2);
}
