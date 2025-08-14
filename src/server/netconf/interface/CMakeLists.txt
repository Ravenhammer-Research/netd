# CMakeLists.txt for interface handlers
# Copyright (c) 2025 Paige Thompson / Raven Hammer Research

# Add subdirectories
add_subdirectory(request)
add_subdirectory(response)
add_subdirectory(type)

# Create interface library
add_library(interface INTERFACE)

# Link libraries
target_link_libraries(interface INTERFACE
    interface_request
    interface_response
    bridge_type
    vlan_type
    lagg_type
    gif_type
    epair_type
    ieee80211_type
)

# Set include directories
target_include_directories(interface INTERFACE
    ${CMAKE_BINARY_DIR}/vendor_install/include
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_SOURCE_DIR}/../..
    ${CMAKE_CURRENT_SOURCE_DIR}/../../..
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../shared
)
