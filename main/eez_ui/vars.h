#ifndef EEZ_LVGL_UI_VARS_H
#define EEZ_LVGL_UI_VARS_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================================
// EEZ Studio will generate enum declarations, flow global variables,
// and native variable getter/setter prototypes here.
// ============================================================================

// Example of what EEZ Studio generates:
//
// enum FlowGlobalVariables {
//     FLOW_GLOBAL_VARIABLE_MY_VAR = 0,
// };
//
// extern const char *get_var_my_string();
// extern void set_var_my_string(const char *value);
// extern int32_t get_var_my_int();
// extern void set_var_my_int(int32_t value);

void init_vars();
void tick_vars();

#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_VARS_H*/
