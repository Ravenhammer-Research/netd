# Handler sources
set(HANDLER_SOURCES
    commit.cpp
    copy.cpp
    delete.cpp
    discard.cpp
    edit.cpp
    hello.cpp
    lock.cpp
    unlock.cpp
    validate.cpp
)

# Add get subdirectory
add_subdirectory(get)

# Add session subdirectory
add_subdirectory(session)

# Create netd_server_netconf_handlers static library
add_library(netd_server_netconf_handlers STATIC ${HANDLER_SOURCES})

# Set include directories
target_include_directories(netd_server_netconf_handlers PRIVATE ${GLOBAL_INCLUDE_DIRS})

# Set compile definitions
target_compile_definitions(netd_server_netconf_handlers PRIVATE
    YANG_DIR="${YANG_DIR}"
)

# Link libraries
target_link_libraries(netd_server_netconf_handlers PRIVATE
    netd_server_netconf_handlers_get
    netd_server_netconf_handlers_session
)
