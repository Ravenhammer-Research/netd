# CMakeLists.txt for gif interface type
# Copyright (c) 2025 Paige Thompson / Raven Hammer Research

# Source files for gif interface type
set(GIF_SOURCES
    request.c
    response.c
)

# Create gif library
add_library(gif_type STATIC ${GIF_SOURCES})

# Set include directories
target_include_directories(gif_type PRIVATE
    ${CMAKE_BINARY_DIR}/vendor_install/include
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_SOURCE_DIR}/../..
    ${CMAKE_CURRENT_SOURCE_DIR}/../../..
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../..
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../../shared
) 