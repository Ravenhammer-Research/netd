# Get handler sources
set(GET_HANDLER_SOURCES
    base.cpp
    config.cpp
    interface.cpp
)

# Create netd_server_netconf_handlers_get static library
add_library(netd_server_netconf_handlers_get STATIC ${GET_HANDLER_SOURCES})

# Set include directories
target_include_directories(netd_server_netconf_handlers_get PRIVATE ${GLOBAL_INCLUDE_DIRS})

# Set compile definitions
target_compile_definitions(netd_server_netconf_handlers_get PRIVATE
    YANG_DIR="${YANG_DIR}"
)
