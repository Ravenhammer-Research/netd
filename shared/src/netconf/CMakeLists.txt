# Create netd_shared_netconf static library
set(NETCONF_SOURCES
    session.cpp
)

add_subdirectory(rpc)

add_library(netd_shared_netconf STATIC ${NETCONF_SOURCES})

# Set include directories
target_include_directories(netd_shared_netconf PRIVATE ${GLOBAL_INCLUDE_DIRS})

# Set compile definitions
target_compile_definitions(netd_shared_netconf PRIVATE
    YANG_DIR="${YANG_DIR}"
)

target_link_libraries(netd_shared_netconf PRIVATE
    netd_shared_netconf_rpc
)