set(LAGG_SOURCES
    set.c
    delete.c
    show.c
)

add_library(freebsd_lagg STATIC ${LAGG_SOURCES})

target_include_directories(freebsd_lagg PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_SOURCE_DIR}/../..
    ${CMAKE_CURRENT_SOURCE_DIR}/../../..
)

target_link_libraries(freebsd_lagg
    netd
) 