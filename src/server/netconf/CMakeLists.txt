# CMakeLists.txt for netconf module
# Copyright (c) 2025 Paige Thompson / Raven Hammer Research

# Add subdirectories
add_subdirectory(interface)
add_subdirectory(route)
add_subdirectory(vrf)

# Source files
set(NETCONF_SOURCES
    netconf.c
    yang.c
    handlers.c
)

# Create netconf library
add_library(netconf STATIC ${NETCONF_SOURCES})

# Link libraries
target_link_libraries(netconf
    shared_debug
    interface
    route_request
    route_response
    vrf_request
    vrf_response
)

# Set include directories
target_include_directories(netconf PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_SOURCE_DIR}/../xml
    ${CMAKE_CURRENT_SOURCE_DIR}/../system
    ${CMAKE_CURRENT_SOURCE_DIR}/../system/freebsd
    ${CMAKE_CURRENT_SOURCE_DIR}/../../shared
    ${CMAKE_CURRENT_SOURCE_DIR}/../../yang
    ${CMAKE_SOURCE_DIR}/src/shared
)
