set(BRIDGE_SOURCES
    show.c
)

add_library(freebsd_bridge STATIC ${BRIDGE_SOURCES})

target_include_directories(freebsd_bridge PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_SOURCE_DIR}/../..
    ${CMAKE_CURRENT_SOURCE_DIR}/../../..
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../shared
)

target_link_libraries(freebsd_bridge
    netd
) 