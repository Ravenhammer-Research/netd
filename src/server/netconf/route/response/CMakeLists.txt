# CMakeLists.txt for route response handlers
# Copyright (c) 2025 Paige Thompson / Raven Hammer Research

# Source files for route response handlers
set(ROUTE_RESPONSE_SOURCES
    get.c
    set.c
)

# Create route response library
add_library(route_response STATIC ${ROUTE_RESPONSE_SOURCES})

# Set include directories
target_include_directories(route_response PRIVATE
    ${CMAKE_BINARY_DIR}/vendor_install/include
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_SOURCE_DIR}/../..
    ${CMAKE_CURRENT_SOURCE_DIR}/../../..
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../..
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../../shared
    ${CMAKE_SOURCE_DIR}/src/shared
)
