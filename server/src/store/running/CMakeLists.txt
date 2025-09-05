# Running store CMakeLists.txt

add_library(running_store
    load.cpp
    commit.cpp
    base.cpp
)

target_include_directories(running_store
    PUBLIC
        ${CMAKE_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/shared/include
        ${LIBYANG_INCLUDE_DIRS}
)

target_link_libraries(running_store
    PUBLIC
        yang
)