# CMakeLists.txt for netconf module
# Copyright (c) 2025 Paige Thompson / Raven Hammer Research

set(NETCONF_SOURCES
    netconf.c
    request.c
    edit.c
    response.c
    route.c
    vrf.c
    yang.c
)

# Add interface subdirectory
add_subdirectory(interface)

# Create netconf library
add_library(netconf STATIC ${NETCONF_SOURCES})

# Set include directories
target_include_directories(netconf PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_SOURCE_DIR}/../xml
    ${CMAKE_CURRENT_SOURCE_DIR}/../system
    ${CMAKE_CURRENT_SOURCE_DIR}/../system/freebsd
    ${CMAKE_CURRENT_SOURCE_DIR}/../../yang
)

# Link dependencies
target_link_libraries(netconf
    netd
    xml
    interface
)



# Install library
install(TARGETS netconf
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
) 