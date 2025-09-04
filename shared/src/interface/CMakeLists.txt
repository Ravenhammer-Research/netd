# Interface source files CMakeLists.txt

# Add interface source files to the shared library
target_sources(netd_shared
    PRIVATE
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
        80211.cpp
)
