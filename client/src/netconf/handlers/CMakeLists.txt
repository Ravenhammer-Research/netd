# Add subdirectories
add_subdirectory(get)
add_subdirectory(session)

# Create netd_client_netconf_handlers static library
add_library(netd_client_netconf_handlers STATIC
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

# Set include directories
target_include_directories(netd_client_netconf_handlers PRIVATE ${GLOBAL_INCLUDE_DIRS})

# Set compile definitions
target_compile_definitions(netd_client_netconf_handlers PRIVATE
    YANG_DIR="${YANG_DIR}"
)

# Link libraries
target_link_libraries(netd_client_netconf_handlers PRIVATE
    netd_shared_netconf
    netd_client_netconf_handlers_get
    netd_client_netconf_handlers_session
)
