# LLDP client library
set(LLDP_SOURCES
    connection.cpp
    discovery.cpp
    service.cpp
    interface.cpp
    neighbor.cpp
    info.cpp
    client.cpp
)

add_library(netd_shared_lldp STATIC ${LLDP_SOURCES})

target_include_directories(netd_shared_lldp PRIVATE ${GLOBAL_INCLUDE_DIRS})

target_link_libraries(netd_shared_lldp
    netd_shared
)

