#ifndef EEZ_LVGL_UI_SCREENS_H
#define EEZ_LVGL_UI_SCREENS_H

#include <lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

// Screens

enum ScreensEnum {
    _SCREEN_ID_FIRST = 1,
    SCREEN_ID_MAIN = 1,
    SCREEN_ID_LEFT_OVEN_PROFILES = 2,
    SCREEN_ID_LEFT_OVEN_SET_TEMP = 3,
    SCREEN_ID_AUDIO_SETTING = 4,
    SCREEN_ID_RTC_SETTING = 5,
    SCREEN_ID_BRIGTHNESS_SETTING = 6,
    _SCREEN_ID_LAST = 6
};

typedef struct _objects_t {
    lv_obj_t *main;
    lv_obj_t *left_oven_profiles;
    lv_obj_t *left_oven_set_temp;
    lv_obj_t *audio_setting;
    lv_obj_t *rtc_setting;
    lv_obj_t *brigthness_setting;
    lv_obj_t *obj0;
    lv_obj_t *obj1;
    lv_obj_t *obj2;
    lv_obj_t *obj3;
    lv_obj_t *obj4;
    lv_obj_t *obj5;
    lv_obj_t *obj6;
    lv_obj_t *obj7;
    lv_obj_t *obj8;
    lv_obj_t *obj9;
    lv_obj_t *obj10;
    lv_obj_t *obj11;
    lv_obj_t *obj12;
    lv_obj_t *obj13;
    lv_obj_t *obj14;
    lv_obj_t *obj15;
    lv_obj_t *obj16;
    lv_obj_t *obj17;
    lv_obj_t *obj18;
    lv_obj_t *obj19;
    lv_obj_t *obj20;
    lv_obj_t *obj21;
    lv_obj_t *obj22;
    lv_obj_t *obj23;
    lv_obj_t *obj24;
    lv_obj_t *obj25;
    lv_obj_t *obj26;
    lv_obj_t *obj27;
    lv_obj_t *obj28;
    lv_obj_t *obj29;
    lv_obj_t *obj30;
    lv_obj_t *obj31;
    lv_obj_t *obj32;
    lv_obj_t *obj33;
    lv_obj_t *btn_set_1;
    lv_obj_t *btn_10;
    lv_obj_t *btn_option_1;
    lv_obj_t *obj34;
    lv_obj_t *obj35;
    lv_obj_t *obj36;
    lv_obj_t *obj37;
    lv_obj_t *obj38;
    lv_obj_t *obj39;
    lv_obj_t *obj40;
    lv_obj_t *obj41;
    lv_obj_t *obj42;
    lv_obj_t *display_label_4;
    lv_obj_t *btn_11;
    lv_obj_t *btn_12;
    lv_obj_t *btn_13;
    lv_obj_t *btn_14;
    lv_obj_t *btn_15;
    lv_obj_t *btn_16;
    lv_obj_t *btn_17;
    lv_obj_t *btn_18;
    lv_obj_t *btn_19;
    lv_obj_t *btn_22;
    lv_obj_t *btn_23;
    lv_obj_t *obj43;
    lv_obj_t *obj44;
    lv_obj_t *btn_20;
    lv_obj_t *btn_21;
    lv_obj_t *display_label_2;
    lv_obj_t *btn_set_2;
    lv_obj_t *btn_set_3;
    lv_obj_t *btn_set_4;
    lv_obj_t *obj45;
    lv_obj_t *obj46;
    lv_obj_t *obj47;
    lv_obj_t *obj48;
    lv_obj_t *btn_set_6;
    lv_obj_t *obj49;
    lv_obj_t *obj50;
    lv_obj_t *display_label_3;
    lv_obj_t *obj51;
    lv_obj_t *obj52;
    lv_obj_t *obj53;
    lv_obj_t *obj54;
} objects_t;

extern objects_t objects;

void create_screen_main();
void tick_screen_main();

void create_screen_left_oven_profiles();
void tick_screen_left_oven_profiles();

void create_screen_left_oven_set_temp();
void tick_screen_left_oven_set_temp();

void create_screen_audio_setting();
void tick_screen_audio_setting();

void create_screen_rtc_setting();
void tick_screen_rtc_setting();

void create_screen_brigthness_setting();
void tick_screen_brigthness_setting();

void tick_screen_by_id(enum ScreensEnum screenId);
void tick_screen(int screen_index);

void create_screens();

#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_SCREENS_H*/