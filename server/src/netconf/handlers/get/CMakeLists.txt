# Get handlers module sources
set(GET_HANDLERS_SOURCES
    base.cpp
    config.cpp
    interface.cpp
)

# Create the netd_get_handlers static library
add_library(netd_get_handlers STATIC ${GET_HANDLERS_SOURCES})

# Link with required libraries
target_link_libraries(netd_get_handlers
    yang
    netconf2
    netd_shared
    netd_freebsd
    netd_get_request
    netd_get_response
)

# Set include directories
target_include_directories(netd_get_handlers PUBLIC
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}>
    $<BUILD_INTERFACE:${LIBYANG_INCLUDE_DIRS}>
    $<BUILD_INTERFACE:${LIBNETCONF2_INCLUDE_DIRS}>
    $<INSTALL_INTERFACE:include>
)

# Set compile definitions
target_compile_definitions(netd_get_handlers PRIVATE
    -DNETD_GET_HANDLERS_EXPORTS
)
