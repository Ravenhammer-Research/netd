add_library(loopback_type STATIC
    request.c
    response.c
)

target_include_directories(loopback_type PRIVATE
    ${CMAKE_SOURCE_DIR}/src/server
    ${CMAKE_SOURCE_DIR}/src/shared
    ${CMAKE_SOURCE_DIR}/vendor_install/include
)

target_link_libraries(loopback_type
    shared_debug
    shared_types
    freebsd_loopback_type
)
