add_library(stf_type STATIC
    request.c
    response.c
)

target_include_directories(stf_type PRIVATE
    ${CMAKE_SOURCE_DIR}/src/server
    ${CMAKE_SOURCE_DIR}/src/shared
    ${CMAKE_SOURCE_DIR}/vendor_install/include
)

target_link_libraries(stf_type
    shared_debug
    shared_types
)
