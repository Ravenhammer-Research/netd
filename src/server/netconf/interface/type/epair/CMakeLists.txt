# CMakeLists.txt for epair interface type
# Copyright (c) 2025 Paige Thompson / Raven Hammer Research

# Source files for epair interface type
set(EPAIR_SOURCES
    request.c
    response.c
)

# Create epair library
add_library(epair_type STATIC ${EPAIR_SOURCES})

# Set include directories
target_include_directories(epair_type PRIVATE
    ${CMAKE_BINARY_DIR}/vendor_install/include
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_SOURCE_DIR}/../..
    ${CMAKE_CURRENT_SOURCE_DIR}/../../..
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../..
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../../shared
) 