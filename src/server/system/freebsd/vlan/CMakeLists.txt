set(VLAN_SOURCES
    set.c
    delete.c
    show.c
)

add_library(freebsd_vlan STATIC ${VLAN_SOURCES})

target_include_directories(freebsd_vlan PRIVATE
    ${CMAKE_SOURCE_DIR}/src/server
    ${CMAKE_SOURCE_DIR}/src/server/system/freebsd
)

target_link_libraries(freebsd_vlan
    netd_common
) 