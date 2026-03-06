# Minimum CMake version required by ESP-IDF
cmake_minimum_required(VERSION 3.16)

# ============================================================================
# EEZ Studio UI Integration (Standalone Flow)
# ============================================================================
# EEZ Studio generates all files into main/eez_ui/ directory, including
# eez-flow.cpp/h (the entire flow engine embedded — no external framework).
#
# Workflow:
#   1. Export from EEZ Studio → get a folder of generated files
#   2. Run: python import_eez_ui.py <export_folder>
#   3. Build: idf.py build flash
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
        ${EEZ_UI_SOURCES}         # All EEZ Studio generated UI files (inc. eez-flow.cpp)

    INCLUDE_DIRS
        "."                       # Current directory (for includes from main.c)
        "eez_ui"                  # EEZ generated UI headers (ui.h, screens.h, eez-flow.h)

    PRIV_REQUIRES
        lvgl                      # LVGL graphics library
        esp_lcd                   # ESP LCD driver framework
        driver                    # ESP-IDF hardware drivers
        spi_flash                 # SPI flash access
)

# Suppress warnings in EEZ generated code (auto-generated, not user-maintained)
target_compile_options(${COMPONENT_LIB} PRIVATE -w)
