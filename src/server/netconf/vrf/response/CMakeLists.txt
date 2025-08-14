# CMakeLists.txt for VRF response handlers
# Copyright (c) 2025 Paige Thompson / Raven Hammer Research

# Source files for VRF response handlers
set(VRF_RESPONSE_SOURCES
    get.c
    set.c
)

# Create VRF response library
add_library(vrf_response STATIC ${VRF_RESPONSE_SOURCES})

# Set include directories
target_include_directories(vrf_response PRIVATE
    ${CMAKE_BINARY_DIR}/vendor_install/include
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_SOURCE_DIR}/../..
    ${CMAKE_CURRENT_SOURCE_DIR}/../../..
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../..
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../../shared
    ${CMAKE_SOURCE_DIR}/src/shared
)
