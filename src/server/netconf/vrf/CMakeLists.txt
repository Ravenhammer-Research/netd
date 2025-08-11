# VRF module for NETCONF server
add_library(vrf STATIC
    set.c
    delete.c
    show.c
    request.c
    response.c
)

target_include_directories(vrf PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_SOURCE_DIR}/../..
    ${CMAKE_CURRENT_SOURCE_DIR}/../../..
    ${CMAKE_CURRENT_SOURCE_DIR}/../../system
    ${CMAKE_CURRENT_SOURCE_DIR}/../../system/freebsd
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../shared
)

target_link_libraries(vrf
    netd
    shared_debug
    route
) 