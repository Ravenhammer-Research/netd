# CMakeLists.txt for VRF handlers
# Copyright (c) 2025 Paige Thompson / Raven Hammer Research

# Add subdirectories
add_subdirectory(request)
add_subdirectory(response)

# Create VRF library
add_library(vrf INTERFACE)

# Link libraries
target_link_libraries(vrf INTERFACE
    vrf_request
    vrf_response
)

# Set include directories
target_include_directories(vrf INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_SOURCE_DIR}/../..
    ${CMAKE_CURRENT_SOURCE_DIR}/../../..
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../shared
)
