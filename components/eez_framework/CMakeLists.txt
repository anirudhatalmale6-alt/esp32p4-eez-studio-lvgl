# ============================================================================
# EEZ Framework — ESP-IDF Component
# ============================================================================
# This wraps the eez-framework library as a proper ESP-IDF component.
# The framework source is in components/eez_framework/eez-framework/src/
# It is included as a git submodule.
# ============================================================================

# Glob all C and C++ sources from the eez-framework source tree
file(GLOB_RECURSE EEZ_FW_SOURCES
    "eez-framework/src/eez/*.cpp"
    "eez-framework/src/eez/*.c"
)

idf_component_register(
    SRCS
        ${EEZ_FW_SOURCES}

    INCLUDE_DIRS
        "eez-framework/src"
        "eez-framework/src/eez/libs/agg"

    PRIV_REQUIRES
        lvgl
)

# Required define for EEZ framework LVGL integration
target_compile_definitions(${COMPONENT_LIB} PUBLIC
    EEZ_FOR_LVGL
    LV_LVGL_H_INCLUDE_SIMPLE
)

# Suppress warnings in the framework (third-party code)
target_compile_options(${COMPONENT_LIB} PRIVATE -w)
