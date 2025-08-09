set(LAGG_SOURCES
    set.c
    delete.c
    show.c
)

add_library(freebsd_lagg STATIC ${LAGG_SOURCES})

target_include_directories(freebsd_lagg PRIVATE
    ${CMAKE_SOURCE_DIR}/src/server
    ${CMAKE_SOURCE_DIR}/src/server/system/freebsd
)

target_link_libraries(freebsd_lagg
    netd_common
) 