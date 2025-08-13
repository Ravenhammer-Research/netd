# CMakeLists.txt for route request handlers
# Copyright (c) 2025 Paige Thompson / Raven Hammer Research

# Source files for route request handlers
set(ROUTE_REQUEST_SOURCES
    get.c
    set.c
)

# Create route request library
add_library(route_request STATIC ${ROUTE_REQUEST_SOURCES})

# Set include directories
target_include_directories(route_request PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_SOURCE_DIR}/../..
    ${CMAKE_CURRENT_SOURCE_DIR}/../../..
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../..
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../../shared
    ${CMAKE_SOURCE_DIR}/src/shared
)
