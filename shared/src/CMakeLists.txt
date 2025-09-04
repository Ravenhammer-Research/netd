# Shared src CMakeLists.txt

# Add subdirectories
add_subdirectory(interface)

# Add source files to the shared library
target_sources(netd_shared
    PRIVATE
        address.cpp
        exception.cpp
        interface.cpp
        logger.cpp
        request.cpp
        response.cpp
        route.cpp
        vrf.cpp
        yang.cpp
        interface/base/ether.cpp
        interface/base/master.cpp
        interface/base/tunnel.cpp
)