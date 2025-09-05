# Response module sources
set(RESPONSE_SOURCES
    config.cpp
    copy.cpp
    delete.cpp
    discard.cpp
    edit.cpp
    lock.cpp
    unlock.cpp
)

# Create the netd_response static library
add_library(netd_response STATIC ${RESPONSE_SOURCES})

# Link with libyang
target_link_libraries(netd_response yang)

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
