# Response module sources
set(RESPONSE_SOURCES
    base.cpp
    copy.cpp
    delete.cpp
    discard.cpp
    edit.cpp
    lock.cpp
    unlock.cpp
    close.cpp
    kill.cpp
    commit.cpp
    validate.cpp
    hello.cpp
)

# Add get subdirectory
add_subdirectory(get)

# Create the netd_response static library
add_library(netd_response STATIC ${RESPONSE_SOURCES})

# Link with libyang and marshalling
target_link_libraries(netd_response yang netd_marshalling)

# Set include directories
target_include_directories(netd_response PUBLIC
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}>
    $<BUILD_INTERFACE:${LIBYANG_INCLUDE_DIRS}>
    $<INSTALL_INTERFACE:include>
)

# Set compile definitions
target_compile_definitions(netd_response PRIVATE
    -DNETD_RESPONSE_EXPORTS
)
