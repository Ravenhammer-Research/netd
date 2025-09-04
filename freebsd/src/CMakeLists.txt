# FreeBSD Source CMakeLists.txt

# Create the FreeBSD implementation library
add_library(netd_freebsd STATIC
    interface/bridge.cpp
    interface/epair.cpp
    interface/ethernet.cpp
    interface/lagg.cpp
    interface/ppp.cpp
    interface/tap.cpp
    interface/tun.cpp
    interface/vlan.cpp
    interface/vxlan.cpp
    interface/wireguard.cpp
    interface/80211.cpp
    vrf.cpp
    route.cpp
)

# Set include directories
target_include_directories(netd_freebsd
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/../..
        ${LIBYANG_INCLUDE_DIRS}
        ${LIBNETCONF2_INCLUDE_DIRS}
)

# Link against system libraries
target_link_libraries(netd_freebsd
    PUBLIC
        netd_shared
        yang
        netconf2
)

# Set compile definitions for FreeBSD
target_compile_definitions(netd_freebsd
    PRIVATE
        _FREEBSD
        __FreeBSD__
)
