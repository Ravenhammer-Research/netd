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
)

# Create server executable
add_executable(netd ${SERVER_SOURCES})

# Set include directories
target_include_directories(netd PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../shared
    ${CMAKE_CURRENT_SOURCE_DIR}/netconf
    ${CMAKE_CURRENT_SOURCE_DIR}/xml
    ${CMAKE_CURRENT_SOURCE_DIR}/system
    ${CMAKE_CURRENT_SOURCE_DIR}/system/freebsd
)

# Link dependencies
target_link_libraries(netd
    shared_debug
    netconf
    xml
    freebsd_system
    expat
    yang
)

# Make target available globally
set_target_properties(netd PROPERTIES GLOBAL TRUE)



 