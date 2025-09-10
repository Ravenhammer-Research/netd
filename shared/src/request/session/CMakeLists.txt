# Session request sources
set(SESSION_REQUEST_SOURCES
    close.cpp
    destroy.cpp
)

# Create netd_shared_request_session static library
add_library(netd_shared_request_session STATIC ${SESSION_REQUEST_SOURCES})

# Set include directories
target_include_directories(netd_shared_request_session PRIVATE ${GLOBAL_INCLUDE_DIRS})

# Set compile definitions
target_compile_definitions(netd_shared_request_session PRIVATE
    YANG_DIR="${YANG_DIR}"
)
