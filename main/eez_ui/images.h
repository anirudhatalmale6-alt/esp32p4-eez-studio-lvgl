#ifndef EEZ_LVGL_UI_IMAGES_H
#define EEZ_LVGL_UI_IMAGES_H

#include <lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

extern const lv_img_dsc_t img_oven_light;
extern const lv_img_dsc_t img_metal_blue_star_logo;
extern const lv_img_dsc_t img_bottom_arrow;
extern const lv_img_dsc_t img_split_screen;
extern const lv_img_dsc_t img_bottom_settings;
extern const lv_img_dsc_t img_bottom_blue_star_logo;
extern const lv_img_dsc_t img_touch_delete;
extern const lv_img_dsc_t img_touch_backspace;
extern const lv_img_dsc_t img_left_oven_active;
extern const lv_img_dsc_t img_left_oven_inactive;
extern const lv_img_dsc_t img_right_oven_active;
extern const lv_img_dsc_t img_right_oven_inactive;
extern const lv_img_dsc_t img_released__bake;
extern const lv_img_dsc_t img_released__broil;
extern const lv_img_dsc_t img_released__clean;
extern const lv_img_dsc_t img_released__convection;
extern const lv_img_dsc_t img_released__convection_bake;
extern const lv_img_dsc_t img_released__convection_roast;
extern const lv_img_dsc_t img_released__dehydrate;
extern const lv_img_dsc_t img_released__proof;
extern const lv_img_dsc_t img_released__roast;
extern const lv_img_dsc_t img_released__stone;

#ifndef EXT_IMG_DESC_T
#define EXT_IMG_DESC_T
typedef struct _ext_img_desc_t {
    const char *name;
    const lv_img_dsc_t *img_dsc;
} ext_img_desc_t;
#endif

extern const ext_img_desc_t images[22];

#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_IMAGES_H*/