# Startup store CMakeLists.txt

add_library(startup_store
    load.cpp
    commit.cpp
    base.cpp
)

target_include_directories(startup_store
    PUBLIC
        ${CMAKE_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/shared/include
        ${LIBYANG_INCLUDE_DIRS}
)

target_link_libraries(startup_store
    PUBLIC
        yang
)