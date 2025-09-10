# Create netd_client_netconf_handlers_session static library
add_library(netd_client_netconf_handlers_session STATIC
    close.cpp
    destroy.cpp
)

# Set include directories
target_include_directories(netd_client_netconf_handlers_session PRIVATE ${GLOBAL_INCLUDE_DIRS})

# Set compile definitions
target_compile_definitions(netd_client_netconf_handlers_session PRIVATE
    YANG_DIR="${YANG_DIR}"
)

# Link libraries
target_link_libraries(netd_client_netconf_handlers_session PRIVATE
    netd_shared_netconf
)
