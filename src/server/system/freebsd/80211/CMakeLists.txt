# CMakeLists.txt for 80211 interface type
# Copyright (c) 2025 Paige Thompson / Raven Hammer Research

add_library(freebsd_80211_type STATIC
    delete.c
    set.c
    show.c
)

# Set include directories
target_include_directories(freebsd_80211_type PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_SOURCE_DIR}/../..
    ${CMAKE_CURRENT_SOURCE_DIR}/../../..
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../shared
)

# Add compile definitions
target_compile_definitions(freebsd_80211_type PUBLIC ${NETD_DEFINES}) 