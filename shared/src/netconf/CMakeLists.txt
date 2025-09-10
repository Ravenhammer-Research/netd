# Create netd_shared_netconf static library
add_library(netd_shared_netconf STATIC
    session.cpp
    rpc.cpp
    transport.cpp
    unix.cpp
    sctp.cpp
    http.cpp
    tls.cpp
    dtls.cpp
)

# Set include directories
target_include_directories(netd_shared_netconf PRIVATE ${GLOBAL_INCLUDE_DIRS})

# Set compile definitions
target_compile_definitions(netd_shared_netconf PRIVATE
    YANG_DIR="${YANG_DIR}"
)
