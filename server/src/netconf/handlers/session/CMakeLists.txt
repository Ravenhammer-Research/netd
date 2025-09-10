# Session handler sources
set(SESSION_HANDLER_SOURCES
    close.cpp
    destroy.cpp
)

# Create netd_server_netconf_handlers_session static library
add_library(netd_server_netconf_handlers_session STATIC ${SESSION_HANDLER_SOURCES})

# Set include directories
target_include_directories(netd_server_netconf_handlers_session PRIVATE ${GLOBAL_INCLUDE_DIRS})

# Set compile definitions
target_compile_definitions(netd_server_netconf_handlers_session PRIVATE
    YANG_DIR="${YANG_DIR}"
)
