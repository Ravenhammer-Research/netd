# CMakeLists.txt for interface response handlers
# Copyright (c) 2025 Paige Thompson / Raven Hammer Research

# Source files for interface response handlers
set(INTERFACE_RESPONSE_SOURCES
    get.c
    set.c
)

# Create interface response library
add_library(interface_response STATIC ${INTERFACE_RESPONSE_SOURCES})

# Set include directories
target_include_directories(interface_response PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_SOURCE_DIR}/../..
    ${CMAKE_CURRENT_SOURCE_DIR}/../../..
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../..
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../../shared
    ${CMAKE_SOURCE_DIR}/src/shared
)
