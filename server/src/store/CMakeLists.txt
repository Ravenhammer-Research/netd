# Store sources
set(STORE_SOURCES
    base.cpp
)

# Add subdirectories
add_subdirectory(candidate)
add_subdirectory(running)
add_subdirectory(startup)

# Create netd_server_store static library
add_library(netd_server_store STATIC ${STORE_SOURCES})

# Set include directories
target_include_directories(netd_server_store PRIVATE ${GLOBAL_INCLUDE_DIRS})

# Set compile definitions
target_compile_definitions(netd_server_store PRIVATE
    YANG_DIR="${YANG_DIR}"
)

# Link libraries
target_link_libraries(netd_server_store PRIVATE
    netd_server_store_candidate
    netd_server_store_running
    netd_server_store_startup
)
