set(INTERFACE_SOURCES
    set.c
    delete.c
    show.c
)

add_library(freebsd_interface STATIC ${INTERFACE_SOURCES})

target_include_directories(freebsd_interface PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_SOURCE_DIR}/../..
    ${CMAKE_CURRENT_SOURCE_DIR}/../../..
)

target_link_libraries(freebsd_interface
    netd
) 