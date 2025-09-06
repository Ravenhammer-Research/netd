# Running store sources
set(RUNNING_STORE_SOURCES
    base.cpp
    commit.cpp
    load.cpp
)

# Create netd_server_store_running static library
add_library(netd_server_store_running STATIC ${RUNNING_STORE_SOURCES})

# Set include directories
target_include_directories(netd_server_store_running PRIVATE ${GLOBAL_INCLUDE_DIRS})

# Set compile definitions
target_compile_definitions(netd_server_store_running PRIVATE
    YANG_DIR="${YANG_DIR}"
)
