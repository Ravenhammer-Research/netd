# Server CMakeLists.txt

# Create server executable target
add_executable(netd_server)

# Set target properties
set_target_properties(netd_server PROPERTIES
    OUTPUT_NAME netd
    VERSION ${PROJECT_VERSION}
)

# Add subdirectories
add_subdirectory(src)

# Link dependencies
target_link_libraries(netd_server
    netd_shared
    netd_netconf
    netd_freebsd
    yang
    netconf2
    pthread
)

# Set library directories
target_link_directories(netd_server
    PRIVATE
        ${LIBYANG_LIBRARY_DIRS}
        ${LIBNETCONF2_LIBRARY_DIRS}
)

# Set include directories
target_include_directories(netd_server
    PRIVATE
        ${CMAKE_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${LIBYANG_INCLUDE_DIRS}
        ${LIBNETCONF2_INCLUDE_DIRS}
)

# Set compile definitions
target_compile_definitions(netd_server
    PRIVATE
        NETD_SERVER
)

# Install rules
install(TARGETS netd_server
    RUNTIME DESTINATION sbin
)
