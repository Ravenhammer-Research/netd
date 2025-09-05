# Handlers module sources
set(HANDLERS_SOURCES
    config.cpp
    copy.cpp
    delete.cpp
    discard.cpp
    edit.cpp
    get.cpp
    lock.cpp
    unlock.cpp
)

# Create the netd_handlers static library
add_library(netd_handlers STATIC ${HANDLERS_SOURCES})

# Link with required libraries
target_link_libraries(netd_handlers
    yang
    netconf2
)

# Set include directories
target_include_directories(netd_handlers PUBLIC
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}>
    $<BUILD_INTERFACE:${LIBYANG_INCLUDE_DIRS}>
    $<BUILD_INTERFACE:${LIBNETCONF2_INCLUDE_DIRS}>
    $<INSTALL_INTERFACE:include>
)

# Set compile definitions
target_compile_definitions(netd_handlers PRIVATE
    -DNETD_HANDLERS_EXPORTS
)
