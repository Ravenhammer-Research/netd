# Netconf sources
set(NETCONF_SOURCES
    base.cpp
    server.cpp
    unix.cpp
)

# Add subdirectories
add_subdirectory(handlers)

# Create netd_server_netconf static library
add_library(netd_server_netconf STATIC ${NETCONF_SOURCES})

# Set include directories
target_include_directories(netd_server_netconf PRIVATE ${GLOBAL_INCLUDE_DIRS})

# Set compile definitions
target_compile_definitions(netd_server_netconf PRIVATE
    YANG_DIR="${YANG_DIR}"
)

# Link libraries
target_link_libraries(netd_server_netconf PRIVATE
    netd_server_netconf_handlers
)
