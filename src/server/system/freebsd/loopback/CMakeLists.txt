set(LOOPBACK_SOURCES
    set.c
    delete.c
    show.c
)

add_library(freebsd_loopback STATIC ${LOOPBACK_SOURCES})

target_include_directories(freebsd_loopback PRIVATE
    ${CMAKE_SOURCE_DIR}/src/server
    ${CMAKE_SOURCE_DIR}/src/server/system/freebsd
)

target_link_libraries(freebsd_loopback
    netd_common
) 