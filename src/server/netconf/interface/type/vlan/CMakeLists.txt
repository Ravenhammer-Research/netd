# CMakeLists.txt for vlan interface type
# Copyright (c) 2025 Paige Thompson / Raven Hammer Research

# Source files for vlan interface type
set(VLAN_SOURCES
    request.c
    response.c
)

# Create vlan library
add_library(vlan_type STATIC ${VLAN_SOURCES})

# Set include directories
target_include_directories(vlan_type PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_SOURCE_DIR}/../..
    ${CMAKE_CURRENT_SOURCE_DIR}/../../..
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../..
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../../shared
) 