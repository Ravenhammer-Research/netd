# RPC stream implementation
add_library(netd_shared_netconf_rpc
  stream.cpp
  rpc.cpp
)

# Set include directories
target_include_directories(netd_shared_netconf_rpc PRIVATE ${GLOBAL_INCLUDE_DIRS})

