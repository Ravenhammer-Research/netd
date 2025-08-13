# CMakeLists.txt for FreeBSD system module
# Copyright (c) 2025 Paige Thompson / Raven Hammer Research

# Add subdirectories for each interface type
add_subdirectory(80211)
add_subdirectory(bridge)
add_subdirectory(epair)
add_subdirectory(gif)
add_subdirectory(interface)
add_subdirectory(lagg)
add_subdirectory(loopback)
add_subdirectory(route)
add_subdirectory(tap)
add_subdirectory(vlan)

# Create the main freebsd library
add_library(freebsd STATIC util.c)

# Set include directories
target_include_directories(freebsd PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_SOURCE_DIR}/../..
    ${CMAKE_CURRENT_SOURCE_DIR}/../../..
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../shared
)

# Add compile definitions
target_compile_definitions(freebsd PUBLIC ${NETD_DEFINES})

# Link dependencies
target_link_libraries(freebsd
    system
    freebsd_80211_type
    freebsd_bridge_type
    freebsd_epair_type
    freebsd_gif_type
    freebsd_interface_type
    freebsd_lagg_type
    freebsd_loopback_type
    freebsd_route_type
    freebsd_tap_type
    freebsd_vlan_type)

