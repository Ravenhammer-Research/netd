# CMakeLists.txt for interface module
# Copyright (c) 2025 Paige Thompson / Raven Hammer Research

set(INTERFACE_SOURCES
    set.c
    delete.c
    show.c
    bridge.c
    vlan.c
    wireless.c
    epair.c
    gif.c
    lagg.c
    loopback.c
    tap.c
    wlan.c
    response.c
    route.c
)

add_library(interface ${INTERFACE_SOURCES})

# Set include directories
target_include_directories(interface PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_SOURCE_DIR}/../..
    ${CMAKE_CURRENT_SOURCE_DIR}/../../..
)

 