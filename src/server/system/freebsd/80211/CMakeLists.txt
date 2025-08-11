set(80211_SOURCES
    set.c
    delete.c
    show.c
)

add_library(freebsd_80211 STATIC ${80211_SOURCES})

target_include_directories(freebsd_80211 PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_SOURCE_DIR}/../..
    ${CMAKE_CURRENT_SOURCE_DIR}/../../..
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../shared
)

target_link_libraries(freebsd_80211
    netd
) 