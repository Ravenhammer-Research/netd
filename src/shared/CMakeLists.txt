# CMakeLists.txt for shared module
# Copyright (c) 2025 Paige Thompson / Raven Hammer Research

# Create shared debug library
add_library(shared_debug STATIC debug.c debug.h)

# Create shared types library (header-only)
add_library(shared_types INTERFACE)

# Set include directories for the shared library
target_include_directories(shared_debug PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Set include directories for the shared types
target_include_directories(shared_types INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Make targets available globally
set_target_properties(shared_debug PROPERTIES GLOBAL TRUE)
set_target_properties(shared_types PROPERTIES GLOBAL TRUE) 