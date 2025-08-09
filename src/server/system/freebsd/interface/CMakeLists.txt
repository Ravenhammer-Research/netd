set(INTERFACE_SOURCES
    set.c
    delete.c
    show.c
)

add_library(freebsd_interface STATIC ${INTERFACE_SOURCES})

target_include_directories(freebsd_interface PRIVATE
    ${CMAKE_SOURCE_DIR}/src/server
    ${CMAKE_SOURCE_DIR}/src/server/system/freebsd
)

target_link_libraries(freebsd_interface
    netd_common
) 