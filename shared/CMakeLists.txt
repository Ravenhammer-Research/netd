# Shared library CMakeLists.txt

# Create shared library target
add_library(netd_shared SHARED)

# Set target properties
set_target_properties(netd_shared PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    OUTPUT_NAME netd_shared
)

# Add subdirectories
add_subdirectory(src)


# Link dependencies
target_link_libraries(netd_shared
    yang
    netconf2
)

# Set library directories
target_link_directories(netd_shared
    PRIVATE
        ${LIBYANG_LIBRARY_DIRS}
        ${LIBNETCONF2_LIBRARY_DIRS}
)

# Set include directories
target_include_directories(netd_shared
    PRIVATE
        ${CMAKE_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${LIBYANG_INCLUDE_DIRS}
        ${LIBNETCONF2_INCLUDE_DIRS}
)

# Set compile definitions
target_compile_definitions(netd_shared
    PRIVATE
        NETD_SHARED_EXPORTS
)

# Install rules
install(TARGETS netd_shared
    EXPORT netd_sharedTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)

install(DIRECTORY include/
    DESTINATION include
    FILES_MATCHING PATTERN "*.hpp"
)

install(EXPORT netd_sharedTargets
    FILE netd_sharedTargets.cmake
    NAMESPACE netd::
    DESTINATION lib/cmake/netd_shared
)
