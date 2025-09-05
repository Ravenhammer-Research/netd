# Request module sources
set(REQUEST_SOURCES
    base.cpp
    get.cpp
    config.cpp
    edit.cpp
    copy.cpp
    delete.cpp
    lock.cpp
    unlock.cpp
    discard.cpp
)

# Create the netd_request static library
add_library(netd_request STATIC ${REQUEST_SOURCES})

# Link with libyang
target_link_libraries(netd_request yang)

# Set include directories
target_include_directories(netd_request PUBLIC
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}>
    $<BUILD_INTERFACE:${LIBYANG_INCLUDE_DIRS}>
    $<INSTALL_INTERFACE:include>
)

# Set compile definitions
target_compile_definitions(netd_request PRIVATE
    -DNETD_REQUEST_EXPORTS
)
