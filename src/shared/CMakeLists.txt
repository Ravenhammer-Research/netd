# CMakeLists.txt for shared module
# Copyright (c) 2025 Paige Thompson / Raven Hammer Research

# Create shared debug library
add_library(shared_debug STATIC debug.c debug.h)

# Set include directories for the shared library
target_include_directories(shared_debug PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Make target available globally
set_target_properties(shared_debug PROPERTIES GLOBAL TRUE) 