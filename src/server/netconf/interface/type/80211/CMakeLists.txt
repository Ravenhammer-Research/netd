add_library(ieee80211_type STATIC
    request.c
    response.c
)

target_include_directories(ieee80211_type PRIVATE
    ${CMAKE_BINARY_DIR}/vendor_install/include
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_SOURCE_DIR}/../..
    ${CMAKE_CURRENT_SOURCE_DIR}/../../..
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../..
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../../shared
)

target_link_libraries(ieee80211_type
    shared_debug
    shared_types
    freebsd_80211_type
) 