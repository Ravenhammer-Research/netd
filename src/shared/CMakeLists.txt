# CMakeLists.txt for shared module
# Copyright (c) 2025 Paige Thompson / Raven Hammer Research

# Create shared debug library
add_library(shared_debug STATIC debug.c debug.h)

# Create shared types library
add_library(shared_types STATIC types.c types.h)

# Set include directories for the shared library
target_include_directories(shared_debug PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Set include directories for the shared types
target_include_directories(shared_types PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Add compile definitions to shared_debug
target_compile_definitions(shared_debug PUBLIC ${NETD_DEFINES})

# Add compile definitions to shared_types
target_compile_definitions(shared_types PUBLIC ${NETD_DEFINES})

# Make targets available globally
set_target_properties(shared_debug PROPERTIES GLOBAL TRUE)
set_target_properties(shared_types PROPERTIES GLOBAL TRUE) 