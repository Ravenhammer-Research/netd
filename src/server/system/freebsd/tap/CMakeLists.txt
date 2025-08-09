set(TAP_SOURCES
    set.c
    delete.c
    show.c
)

add_library(freebsd_tap STATIC ${TAP_SOURCES})

target_include_directories(freebsd_tap PRIVATE
    ${CMAKE_SOURCE_DIR}/src/server
    ${CMAKE_SOURCE_DIR}/src/server/system/freebsd
)

target_link_libraries(freebsd_tap
    netd_common
) 