# Add subdirectories
add_subdirectory(handlers)

# Create netd_server_netconf static library
add_library(netd_server_netconf STATIC
    base.cpp
    server.cpp
    request.cpp
    response.cpp
    session.cpp
)

# Set include directories
target_include_directories(netd_server_netconf PRIVATE ${GLOBAL_INCLUDE_DIRS})

# Set compile definitions
target_compile_definitions(netd_server_netconf PRIVATE
    YANG_DIR="${YANG_DIR}"
)

# Add LLDP-specific definitions if LLDP is found
if(LLDP_FOUND)
    target_compile_definitions(netd_server_netconf PRIVATE NETD_OUI="00:1b:21")
endif()

# Link libraries
target_link_libraries(netd_server_netconf PRIVATE
    netd_shared_netconf
    netd_server_netconf_handlers
)
