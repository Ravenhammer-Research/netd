set(ROUTE_SOURCES
    set.c
    delete.c
    show.c
)

add_library(freebsd_route STATIC ${ROUTE_SOURCES})

target_include_directories(freebsd_route PRIVATE
    ${CMAKE_SOURCE_DIR}/src/server
    ${CMAKE_SOURCE_DIR}/src/server/system/freebsd
)

target_link_libraries(freebsd_route
    netd_common
) 