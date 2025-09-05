# Store module CMakeLists.txt

# Base store library
add_library(store_base
    base.cpp
)

target_include_directories(store_base
    PUBLIC
        ${CMAKE_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/shared/include
        ${LIBYANG_INCLUDE_DIRS}
)

target_link_libraries(store_base
    PUBLIC
        yang
)

# Add subdirectories
add_subdirectory(startup)
add_subdirectory(candidate)
add_subdirectory(running)