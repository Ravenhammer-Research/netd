# CMakeLists.txt for route handlers
# Copyright (c) 2025 Paige Thompson / Raven Hammer Research

# Add subdirectories
add_subdirectory(request)
add_subdirectory(response)

# Create route library
add_library(route INTERFACE)

# Link libraries
target_link_libraries(route INTERFACE
    route_request
    route_response
)

# Set include directories
target_include_directories(route INTERFACE
    ${CMAKE_BINARY_DIR}/vendor_install/include
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_SOURCE_DIR}/../..
    ${CMAKE_CURRENT_SOURCE_DIR}/../../..
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../shared
)
