# Add subdirectories
add_subdirectory(handlers)

# Netconf module sources
set(NETCONF_SOURCES
    base.cpp
    server.cpp
    unix.cpp
)

# Create the netd_netconf static library
add_library(netd_netconf STATIC ${NETCONF_SOURCES})

# Link with required libraries
target_link_libraries(netd_netconf
    yang
    netconf2
    netd_handlers
)

# Set include directories
target_include_directories(netd_netconf PUBLIC
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}>
    $<BUILD_INTERFACE:${LIBYANG_INCLUDE_DIRS}>
    $<BUILD_INTERFACE:${LIBNETCONF2_INCLUDE_DIRS}>
    $<INSTALL_INTERFACE:include>
)

# Set compile definitions
target_compile_definitions(netd_netconf PRIVATE
    -DNETD_NETCONF_EXPORTS
)
