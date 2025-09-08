# Get request sources
set(GET_REQUEST_SOURCES
    base.cpp
    config.cpp
    yanglib.cpp
)

# Create netd_shared_request_get static library
add_library(netd_shared_request_get STATIC ${GET_REQUEST_SOURCES})

# Set include directories
target_include_directories(netd_shared_request_get PRIVATE ${GLOBAL_INCLUDE_DIRS})

# Set compile definitions
target_compile_definitions(netd_shared_request_get PRIVATE
    YANG_DIR="${YANG_DIR}"
)
