# CMakeLists.txt for vlan interface type
# Copyright (c) 2025 Paige Thompson / Raven Hammer Research

add_library(freebsd_vlan_type STATIC
    delete.c
    set.c
    show.c
)

# Set include directories
target_include_directories(freebsd_vlan_type PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_SOURCE_DIR}/../..
    ${CMAKE_CURRENT_SOURCE_DIR}/../../..
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../shared
) 