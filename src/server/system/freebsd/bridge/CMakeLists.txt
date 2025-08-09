set(BRIDGE_SOURCES
    show.c
)

add_library(freebsd_bridge STATIC ${BRIDGE_SOURCES})

target_include_directories(freebsd_bridge PRIVATE
    ${CMAKE_SOURCE_DIR}/src/server
    ${CMAKE_SOURCE_DIR}/src/server/system/freebsd
)

target_link_libraries(freebsd_bridge
    netd_common
) 