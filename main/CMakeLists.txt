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

# Collect WAV files for embedding
file(GLOB AUDIO_FILES "${CMAKE_CURRENT_SOURCE_DIR}/audio/*.wav")

# Register the main component with ESP-IDF
idf_component_register(
    SRCS
        "main.c"                  # Main application (display, touch, app_main)
        "beep.c"                  # Audio beep module (I2S + MAX98357A)
        "rtc_clock.c"             # RTC clock module (32.768 kHz XTAL + NVS)
        "oven_timer.c"            # Countdown timers, delayed start, cooking time
        "prizer_bus.c"            # Prizer bus UART communication (ported from XMC1402)
        "actions_impl.c"          # EEZ user action implementations
        "vars_impl.c"             # EEZ native variable implementations
        ${EEZ_UI_SOURCES}         # All EEZ Studio generated UI files (inc. eez-flow.cpp)

    INCLUDE_DIRS
        "."                       # Current directory (for includes from main.c)
        "eez_ui"                  # EEZ generated UI headers (ui.h, screens.h, eez-flow.h)

    EMBED_FILES
        ${AUDIO_FILES}            # WAV files embedded in flash (beep1.wav, beep2.wav)

    PRIV_REQUIRES
        lvgl                      # LVGL graphics library
        esp_lcd                   # ESP LCD driver framework
        driver                    # ESP-IDF hardware drivers
        esp_driver_i2s            # I2S driver for MAX98357A audio
        esp_driver_uart           # UART driver for Prizer bus communication
        spi_flash                 # SPI flash access
        nvs_flash                 # Non-volatile storage (volume persistence)
)

# Suppress warnings in EEZ generated code (auto-generated, not user-maintained)
target_compile_options(${COMPONENT_LIB} PRIVATE -w)
