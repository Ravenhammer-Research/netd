# CMakeLists.txt for system module
# Copyright (c) 2025 Paige Thompson / Raven Hammer Research

# Add subdirectories
add_subdirectory(freebsd)

# Create system utility library
add_library(system STATIC util.c)

# Set include directories
target_include_directories(system PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_SOURCE_DIR}/../..
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../shared
)

# Add compile definitions
target_compile_definitions(system PUBLIC ${NETD_DEFINES})

# Link dependencies
target_link_libraries(system
    shared_debug
    freebsd
) 