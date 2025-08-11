# Route module for NETCONF server
add_library(route STATIC
    set.c
    delete.c
    show.c
)

target_include_directories(route PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_SOURCE_DIR}/../..
    ${CMAKE_CURRENT_SOURCE_DIR}/../../..
    ${CMAKE_CURRENT_SOURCE_DIR}/../../system
    ${CMAKE_CURRENT_SOURCE_DIR}/../../system/freebsd
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../shared
)

target_link_libraries(route
    netd
    shared_debug
) 