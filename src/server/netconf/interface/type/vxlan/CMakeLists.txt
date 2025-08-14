add_library(vxlan_type STATIC
    request.c
    response.c
)

target_include_directories(vxlan_type PRIVATE
    ${CMAKE_SOURCE_DIR}/src/server
    ${CMAKE_SOURCE_DIR}/src/shared
    ${CMAKE_SOURCE_DIR}/vendor_install/include
)

target_link_libraries(vxlan_type
    shared_debug
    shared_types
)
