add_library(ovpn_type STATIC
    request.c
    response.c
)

target_include_directories(ovpn_type PRIVATE
    ${CMAKE_SOURCE_DIR}/src/server
    ${CMAKE_SOURCE_DIR}/src/shared
    ${CMAKE_SOURCE_DIR}/vendor_install/include
)

target_link_libraries(ovpn_type
    shared_debug
    shared_types
)
