# CMakeLists.txt for server module
# Copyright (c) 2025 Paige Thompson / Raven Hammer Research

set(ENV{PKG_CONFIG_PATH} "/usr/local/lib/pkgconfig:$ENV{PKG_CONFIG_PATH}")

# Find required packages
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBNETCONF2 REQUIRED libnetconf2)
pkg_check_modules(LIBYANG REQUIRED libyang)

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
    ${LIBNETCONF2_INCLUDE_DIRS}
    ${LIBYANG_INCLUDE_DIRS}
)

# Add compile definitions
target_compile_definitions(netd PRIVATE ${NETD_DEFINES})

# Link dependencies
target_link_libraries(netd
    shared_debug
    shared_types
    netconf
    system
    ${LIBNETCONF2_LIBRARIES}
    ${LIBYANG_LIBRARIES}
)

# Set compile flags
target_compile_options(netd PRIVATE ${LIBNETCONF2_CFLAGS_OTHER})
target_compile_options(netd PRIVATE ${LIBYANG_CFLAGS_OTHER})

# Make target available globally
set_target_properties(netd PROPERTIES GLOBAL TRUE)



 