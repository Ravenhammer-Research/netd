# FreeBSD interface sources
set(FREEBSD_INTERFACE_SOURCES
    80211.cpp
    bridge.cpp
    epair.cpp
    ethernet.cpp
    lagg.cpp
    ppp.cpp
    tap.cpp
    tun.cpp
    vlan.cpp
    vxlan.cpp
    wireguard.cpp
)

# Create netd_freebsd_interface static library
add_library(netd_freebsd_interface STATIC ${FREEBSD_INTERFACE_SOURCES})

# Set include directories
target_include_directories(netd_freebsd_interface PRIVATE ${GLOBAL_INCLUDE_DIRS})

# Set compile definitions
target_compile_definitions(netd_freebsd_interface PRIVATE
    YANG_DIR="${YANG_DIR}"
)
