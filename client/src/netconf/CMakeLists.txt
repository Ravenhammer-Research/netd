# Add subdirectories
add_subdirectory(handlers)

# NETCONF client source files
set(NETCONF_CLIENT_SOURCES
  base.cpp
  client.cpp
  rpc.cpp
  reply.cpp
)

# Create the netconf client library
add_library(netd_client_netconf STATIC ${NETCONF_CLIENT_SOURCES})

# Set target properties
target_include_directories(netd_client_netconf PRIVATE ${GLOBAL_INCLUDE_DIRS})

# Set compile definitions
target_compile_definitions(netd_client_netconf PRIVATE
    YANG_DIR="${YANG_DIR}"
)

# Link libraries
target_link_libraries(netd_client_netconf PRIVATE
    netd_shared_netconf
    netd_client_netconf_handlers
)

