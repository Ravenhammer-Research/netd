# CMakeLists.txt for bridge interface type
# Copyright (c) 2025 Paige Thompson / Raven Hammer Research

# Source files for bridge interface type
set(BRIDGE_SOURCES
    request.c
    response.c
)

# Create bridge library
add_library(bridge_type STATIC ${BRIDGE_SOURCES})

# Set include directories
target_include_directories(bridge_type PRIVATE
    ${CMAKE_BINARY_DIR}/vendor_install/include
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_SOURCE_DIR}/../..
    ${CMAKE_CURRENT_SOURCE_DIR}/../../..
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../..
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../../shared
)

# Link against FreeBSD bridge library
target_link_libraries(bridge_type
    freebsd_bridge_type
)
