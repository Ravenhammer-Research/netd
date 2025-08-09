# CMakeLists.txt for FreeBSD system module
# Copyright (c) 2025 Paige Thompson / Raven Hammer Research

set(FREEBSD_SOURCES
    system.c
    interface.c
    route.c
    bridge.c
    epair.c
    gif.c
    lagg.c
    loopback.c
    tap.c
    vlan.c
    wireless.c
)

# Create freebsd system library
add_library(freebsd_system STATIC ${FREEBSD_SOURCES})

# Set include directories
target_include_directories(freebsd_system PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)
target_include_directories(freebsd_system PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_SOURCE_DIR}/../..
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