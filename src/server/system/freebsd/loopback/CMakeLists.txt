set(LOOPBACK_SOURCES
    set.c
    delete.c
    show.c
)

add_library(freebsd_loopback STATIC ${LOOPBACK_SOURCES})

target_include_directories(freebsd_loopback PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_SOURCE_DIR}/../..
    ${CMAKE_CURRENT_SOURCE_DIR}/../../..
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../shared
)

target_link_libraries(freebsd_loopback
    netd
) 