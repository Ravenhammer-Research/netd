# Marshalling sources
set(MARSHALLING_SOURCES
    error.cpp
    filter.cpp
)

# Create netd_shared_marshalling static library
add_library(netd_shared_marshalling STATIC ${MARSHALLING_SOURCES})

# Set include directories
target_include_directories(netd_shared_marshalling PRIVATE ${GLOBAL_INCLUDE_DIRS})

# Set compile definitions
target_compile_definitions(netd_shared_marshalling PRIVATE
    YANG_DIR="${YANG_DIR}"
)
