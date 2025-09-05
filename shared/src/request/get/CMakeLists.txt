# Get request module sources
set(GET_REQUEST_SOURCES
    base.cpp
    config.cpp
)

# Create the netd_get_request static library
add_library(netd_get_request STATIC ${GET_REQUEST_SOURCES})

# Link with libyang
target_link_libraries(netd_get_request yang)

# Set include directories
target_include_directories(netd_get_request PUBLIC
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}>
    $<BUILD_INTERFACE:${LIBYANG_INCLUDE_DIRS}>
    $<INSTALL_INTERFACE:include>
)

# Set compile definitions
target_compile_definitions(netd_get_request PRIVATE
    -DNETD_GET_REQUEST_EXPORTS
)
