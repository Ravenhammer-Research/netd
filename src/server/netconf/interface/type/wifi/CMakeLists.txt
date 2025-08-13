# CMakeLists.txt for wifi interface type
# Copyright (c) 2025 Paige Thompson / Raven Hammer Research

# Source files for wifi interface type
set(WIFI_SOURCES
    request.c
    response.c
)

# Create wifi library
add_library(wifi_type STATIC ${WIFI_SOURCES})

# Set include directories
target_include_directories(wifi_type PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_SOURCE_DIR}/../..
    ${CMAKE_CURRENT_SOURCE_DIR}/../../..
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../..
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../../shared
) 