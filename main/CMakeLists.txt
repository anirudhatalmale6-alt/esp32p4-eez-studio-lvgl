# Minimum CMake version required by ESP-IDF
cmake_minimum_required(VERSION 3.16)

# ============================================================================
# EEZ Studio UI Integration
# ============================================================================
# EEZ Studio generates files into main/eez_ui/ directory.
# This CMakeLists auto-discovers all .c and .cpp files in that folder.
# After exporting from EEZ Studio, just run: python import_eez_ui.py <export_dir>
# Then build normally with: idf.py build flash
# ============================================================================

# Auto-discover all EEZ generated C/C++ source files
file(GLOB_RECURSE EEZ_UI_SOURCES
    "eez_ui/*.c"
    "eez_ui/*.cpp"
)

# Register the main component with ESP-IDF
idf_component_register(
    SRCS
        "main.c"                  # Main application (display, touch, app_main)
        ${EEZ_UI_SOURCES}         # All EEZ Studio generated UI files

    INCLUDE_DIRS
        "."                       # Current directory (for includes from main.c)
        "eez_ui"                  # EEZ generated UI headers (ui.h, screens.h, etc.)

    PRIV_REQUIRES
        lvgl                      # LVGL graphics library
        esp_lcd                   # ESP LCD driver framework
        driver                    # ESP-IDF hardware drivers
        spi_flash                 # SPI flash access
        eez_framework             # EEZ Framework for flow support
)

# EEZ-generated code uses <lvgl/lvgl.h> includes — add the parent of LVGL's include path
# so that both #include "lvgl.h" and #include <lvgl/lvgl.h> resolve correctly.
# Also define EEZ_FOR_LVGL and LV_LVGL_H_INCLUDE_SIMPLE for eez-framework compatibility.
target_compile_definitions(${COMPONENT_LIB} PUBLIC
    EEZ_FOR_LVGL
    LV_LVGL_H_INCLUDE_SIMPLE
)

# Suppress warnings in EEZ generated code (it's auto-generated, not user-maintained)
target_compile_options(${COMPONENT_LIB} PRIVATE -w)
