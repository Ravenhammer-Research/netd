# Get response sources
set(GET_RESPONSE_SOURCES
    base.cpp
    config.cpp
    library.cpp
)

# Create netd_shared_response_get static library
add_library(netd_shared_response_get STATIC ${GET_RESPONSE_SOURCES})

# Set include directories
target_include_directories(netd_shared_response_get PRIVATE ${GLOBAL_INCLUDE_DIRS})

# Set compile definitions
target_compile_definitions(netd_shared_response_get PRIVATE
    YANG_DIR="${YANG_DIR}"
)
