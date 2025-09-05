# Candidate store CMakeLists.txt

add_library(candidate_store
    commit.cpp
    base.cpp
)

target_include_directories(candidate_store
    PUBLIC
        ${CMAKE_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/shared/include
        ${LIBYANG_INCLUDE_DIRS}
)

target_link_libraries(candidate_store
    PUBLIC
        yang
)