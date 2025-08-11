# CMakeLists.txt for FreeBSD system module
# Copyright (c) 2025 Paige Thompson / Raven Hammer Research

# Add subdirectories
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

# Collect all FreeBSD system source files
file(GLOB_RECURSE FREEBSD_SOURCES
    "*.c"
    "80211/*.c"
    "bridge/*.c"
    "epair/*.c"
    "gif/*.c"
    "interface/*.c"
    "lagg/*.c"
    "loopback/*.c"
    "route/*.c"
    "tap/*.c"
    "vlan/*.c"
)

# Create freebsd system library
add_library(freebsd_system STATIC ${FREEBSD_SOURCES})

# Set include directories
target_include_directories(freebsd_system PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_SOURCE_DIR}/../..
    ${CMAKE_CURRENT_SOURCE_DIR}/../../..
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../..
    ${CMAKE_SOURCE_DIR}/example/usr/include
    ${CMAKE_CURRENT_SOURCE_DIR}/80211
    ${CMAKE_CURRENT_SOURCE_DIR}/bridge
    ${CMAKE_CURRENT_SOURCE_DIR}/epair
    ${CMAKE_CURRENT_SOURCE_DIR}/gif
    ${CMAKE_CURRENT_SOURCE_DIR}/interface
    ${CMAKE_CURRENT_SOURCE_DIR}/lagg
    ${CMAKE_CURRENT_SOURCE_DIR}/loopback
    ${CMAKE_CURRENT_SOURCE_DIR}/route
    ${CMAKE_CURRENT_SOURCE_DIR}/tap
    ${CMAKE_CURRENT_SOURCE_DIR}/vlan
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../shared
)

# Link dependencies
target_link_libraries(freebsd_system
    netd
)

# Install library
install(TARGETS freebsd_system
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
) 