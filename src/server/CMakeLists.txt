# CMakeLists.txt for server module
# Copyright (c) 2025 Paige Thompson / Raven Hammer Research

# Add subdirectories
add_subdirectory(netconf)
add_subdirectory(xml)
add_subdirectory(system)

set(SERVER_SOURCES
    main.c
    netd.h
    utils.c
    config.c
    transaction.c
    debug.c
)

# Create server executable
add_executable(netd ${SERVER_SOURCES})

# Set include directories
target_include_directories(netd PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_SOURCE_DIR}/../..
)

# Link dependencies
target_link_libraries(netd
    netconf
    xml
    freebsd_system
)

# Make target available globally
set_target_properties(netd PROPERTIES GLOBAL TRUE)



 