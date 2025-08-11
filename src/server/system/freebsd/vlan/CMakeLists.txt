set(VLAN_SOURCES
    set.c
    delete.c
    show.c
)

add_library(freebsd_vlan STATIC ${VLAN_SOURCES})

target_include_directories(freebsd_vlan PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_SOURCE_DIR}/../..
    ${CMAKE_CURRENT_SOURCE_DIR}/../../..
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../shared
)

target_link_libraries(freebsd_vlan
    netd
) 