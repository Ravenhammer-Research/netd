add_library(arc_type STATIC
    request.c
    response.c
)

target_include_directories(arc_type PRIVATE
    ${CMAKE_SOURCE_DIR}/src/server
    ${CMAKE_SOURCE_DIR}/src/shared
    ${CMAKE_SOURCE_DIR}/vendor_install/include
)

target_link_libraries(arc_type
    shared_debug
    shared_types
)
