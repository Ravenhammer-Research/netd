set(WIRELESS_SOURCES
    set.c
    delete.c
    show.c
)

add_library(freebsd_80211 STATIC ${WIRELESS_SOURCES})

target_include_directories(freebsd_80211 PRIVATE
    ${CMAKE_SOURCE_DIR}/src/server
    ${CMAKE_SOURCE_DIR}/src/server/system/freebsd
)

target_link_libraries(freebsd_80211
    netd_common
) 