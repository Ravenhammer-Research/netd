# CMakeLists.txt for VRF request handlers
# Copyright (c) 2025 Paige Thompson / Raven Hammer Research

# Source files for VRF request handlers
set(VRF_REQUEST_SOURCES
    get.c
)

# Create VRF request library
add_library(vrf_request STATIC ${VRF_REQUEST_SOURCES})

# Set include directories
target_include_directories(vrf_request PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_SOURCE_DIR}/../..
    ${CMAKE_CURRENT_SOURCE_DIR}/../../..
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../..
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../../shared
    ${CMAKE_SOURCE_DIR}/src/shared
)
