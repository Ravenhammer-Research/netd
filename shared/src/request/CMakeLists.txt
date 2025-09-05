# Request module sources
set(REQUEST_SOURCES
    base.cpp
    edit.cpp
    copy.cpp
    delete.cpp
    lock.cpp
    unlock.cpp
    discard.cpp
    close.cpp
    kill.cpp
    commit.cpp
    validate.cpp
    hello.cpp
)

# Add get subdirectory
add_subdirectory(get)

# Create the netd_request static library
add_library(netd_request STATIC ${REQUEST_SOURCES})

# Link with libyang and marshalling
target_link_libraries(netd_request yang netd_marshalling)

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
