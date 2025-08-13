# CMakeLists.txt for interface request handlers
# Copyright (c) 2025 Paige Thompson / Raven Hammer Research

# Source files for interface request handlers
set(INTERFACE_REQUEST_SOURCES
    get.c
    set.c
)

# Create interface request library
add_library(interface_request STATIC ${INTERFACE_REQUEST_SOURCES})

# Set include directories
target_include_directories(interface_request PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_SOURCE_DIR}/../..
    ${CMAKE_CURRENT_SOURCE_DIR}/../../..
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../..
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../../shared
    ${CMAKE_SOURCE_DIR}/src/shared
)
