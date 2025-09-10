# Request sources
set(REQUEST_SOURCES
    base.cpp
    commit.cpp
    copy.cpp
    delete.cpp
    discard.cpp
    edit.cpp
    hello.cpp
    lock.cpp
    unlock.cpp
    validate.cpp
)

# Add get subdirectory
add_subdirectory(get)

# Add session subdirectory
add_subdirectory(session)

# Create netd_shared_request static library
add_library(netd_shared_request STATIC ${REQUEST_SOURCES})

# Set include directories
target_include_directories(netd_shared_request PRIVATE ${GLOBAL_INCLUDE_DIRS})

# Set compile definitions
target_compile_definitions(netd_shared_request PRIVATE
    YANG_DIR="${YANG_DIR}"
)

# Link sub-libraries
target_link_libraries(netd_shared_request PRIVATE
    netd_shared_request_get
    netd_shared_request_session
)
