set(TAP_SOURCES
    set.c
    delete.c
    show.c
)

add_library(freebsd_tap STATIC ${TAP_SOURCES})

target_include_directories(freebsd_tap PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_SOURCE_DIR}/../..
    ${CMAKE_CURRENT_SOURCE_DIR}/../../..
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../shared
)

target_link_libraries(freebsd_tap
    netd
) 