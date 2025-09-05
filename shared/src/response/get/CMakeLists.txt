# Get response module sources
set(GET_RESPONSE_SOURCES
    base.cpp
    config.cpp
)

# Create the netd_get_response static library
add_library(netd_get_response STATIC ${GET_RESPONSE_SOURCES})

# Link with libyang
target_link_libraries(netd_get_response yang)

# Set include directories
target_include_directories(netd_get_response PUBLIC
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}>
    $<BUILD_INTERFACE:${LIBYANG_INCLUDE_DIRS}>
    $<INSTALL_INTERFACE:include>
)

# Set compile definitions
target_compile_definitions(netd_get_response PRIVATE
    -DNETD_GET_RESPONSE_EXPORTS
)
