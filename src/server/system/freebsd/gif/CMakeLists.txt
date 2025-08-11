set(GIF_SOURCES
    set.c
    delete.c
    show.c
)

add_library(freebsd_gif STATIC ${GIF_SOURCES})

target_include_directories(freebsd_gif PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_SOURCE_DIR}/../..
    ${CMAKE_CURRENT_SOURCE_DIR}/../../..
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../shared
)

target_link_libraries(freebsd_gif
    netd
) 