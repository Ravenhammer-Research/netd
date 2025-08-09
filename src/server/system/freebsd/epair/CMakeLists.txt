set(EPAIR_SOURCES
    set.c
    delete.c
    show.c
)

add_library(freebsd_epair STATIC ${EPAIR_SOURCES})

target_include_directories(freebsd_epair PRIVATE
    ${CMAKE_SOURCE_DIR}/src/server
    ${CMAKE_SOURCE_DIR}/src/server/system/freebsd
)

target_link_libraries(freebsd_epair
    netd_common
) 