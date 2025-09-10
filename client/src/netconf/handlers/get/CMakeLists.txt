# Create netd_client_netconf_handlers_get static library
add_library(netd_client_netconf_handlers_get STATIC
    base.cpp
    config.cpp
    interface.cpp
)

# Set include directories
target_include_directories(netd_client_netconf_handlers_get PRIVATE ${GLOBAL_INCLUDE_DIRS})

# Set compile definitions
target_compile_definitions(netd_client_netconf_handlers_get PRIVATE
    YANG_DIR="${YANG_DIR}"
)

# Link libraries
target_link_libraries(netd_client_netconf_handlers_get PRIVATE
    netd_shared_netconf
)
