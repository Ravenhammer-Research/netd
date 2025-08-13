# CMakeLists.txt for lagg interface type
# Copyright (c) 2025 Paige Thompson / Raven Hammer Research

# Source files for lagg interface type
set(LAGG_SOURCES
    request.c
    response.c
)

# Create lagg library
add_library(lagg_type STATIC ${LAGG_SOURCES})

# Set include directories
target_include_directories(lagg_type PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_SOURCE_DIR}/../..
    ${CMAKE_CURRENT_SOURCE_DIR}/../../..
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../..
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../../shared
) 