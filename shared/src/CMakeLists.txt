# Shared src CMakeLists.txt

# Add subdirectories
add_subdirectory(interface)
add_subdirectory(request)
add_subdirectory(response)

# Add source files to the shared library
target_sources(netd_shared
    PRIVATE
        address.cpp
        exception.cpp
        logger.cpp
        route.cpp
        vrf.cpp
        yang.cpp
        interface/base/ether.cpp
        interface/base/master.cpp
        interface/base/tunnel.cpp
)