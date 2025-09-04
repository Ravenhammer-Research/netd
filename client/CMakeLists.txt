# Client CMakeLists.txt

# Create client executable target
add_executable(netd_client)

# Set target properties
set_target_properties(netd_client PROPERTIES
    OUTPUT_NAME net
    VERSION ${PROJECT_VERSION}
)

# Add subdirectories
add_subdirectory(src)
add_subdirectory(parser)

# Link dependencies
target_link_libraries(netd_client
    netd_shared
    yang
    netconf2
    ${CURSES_LIBRARIES}
)

# Set library directories
target_link_directories(netd_client
    PRIVATE
        ${LIBYANG_LIBRARY_DIRS}
        ${LIBNETCONF2_LIBRARY_DIRS}
)

# Set include directories
target_include_directories(netd_client
    PRIVATE
        ${CMAKE_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${LIBYANG_INCLUDE_DIRS}
        ${LIBNETCONF2_INCLUDE_DIRS}
)

# Set compile definitions
target_compile_definitions(netd_client
    PRIVATE
        NETD_CLIENT
)

# Install rules
install(TARGETS netd_client
    RUNTIME DESTINATION bin
)
