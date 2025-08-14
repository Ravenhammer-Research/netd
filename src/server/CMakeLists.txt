# CMakeLists.txt for server module
# Copyright (c) 2025 Paige Thompson / Raven Hammer Research

# Add subdirectories
add_subdirectory(netconf)
add_subdirectory(system)

set(SERVER_SOURCES
    main.c
    netd.h
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

# Add compile definitions
target_compile_definitions(netd PRIVATE ${NETD_DEFINES})

# Link dependencies
target_link_libraries(netd
    shared_debug
    shared_types
    netconf
    system
    ${LIBYANG_LIBRARIES}
    ${LIBNETCONF2_LIBRARIES}
)

# Make netd depend on vendor_install to ensure libraries are built first
if(TARGET vendor_install)
    add_dependencies(netd vendor_install)
endif()

# Make all server targets depend on vendor_install
if(TARGET vendor_install)
    add_dependencies(netconf vendor_install)
    add_dependencies(system vendor_install)
endif()

# Make target available globally
set_target_properties(netd PROPERTIES GLOBAL TRUE)



 