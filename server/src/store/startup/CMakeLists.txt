# Startup store sources
set(STARTUP_STORE_SOURCES
    base.cpp
    commit.cpp
    load.cpp
)

# Create netd_server_store_startup static library
add_library(netd_server_store_startup STATIC ${STARTUP_STORE_SOURCES})

# Set include directories
target_include_directories(netd_server_store_startup PRIVATE ${GLOBAL_INCLUDE_DIRS})

# Set compile definitions
target_compile_definitions(netd_server_store_startup PRIVATE
    YANG_DIR="${YANG_DIR}"
)
