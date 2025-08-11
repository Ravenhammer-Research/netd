set(EPAIR_SOURCES
    set.c
    delete.c
    show.c
)

add_library(freebsd_epair STATIC ${EPAIR_SOURCES})

target_include_directories(freebsd_epair PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_SOURCE_DIR}/../..
    ${CMAKE_CURRENT_SOURCE_DIR}/../../..
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../shared
)

target_link_libraries(freebsd_epair
    netd
) 