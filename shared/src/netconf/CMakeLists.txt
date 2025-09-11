# Create netd_shared_netconf static library
set(NETCONF_SOURCES
    session.cpp
    rpc.cpp
)

add_library(netd_shared_netconf STATIC ${NETCONF_SOURCES})

# Set include directories
target_include_directories(netd_shared_netconf PRIVATE ${GLOBAL_INCLUDE_DIRS})

# Set compile definitions
target_compile_definitions(netd_shared_netconf PRIVATE
    YANG_DIR="${YANG_DIR}"
)
