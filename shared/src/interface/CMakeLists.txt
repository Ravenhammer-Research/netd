# Interface sources
set(INTERFACE_SOURCES
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

# Add subdirectories
add_subdirectory(base)

# Create netd_shared_interface static library
add_library(netd_shared_interface STATIC ${INTERFACE_SOURCES})

# Set include directories
target_include_directories(netd_shared_interface PRIVATE ${GLOBAL_INCLUDE_DIRS})

# Set compile definitions
target_compile_definitions(netd_shared_interface PRIVATE
    YANG_DIR="${YANG_DIR}"
)

# Link base interface library
target_link_libraries(netd_shared_interface PRIVATE
    netd_shared_interface_base
)
