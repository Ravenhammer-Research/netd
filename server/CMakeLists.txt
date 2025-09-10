# Server sources
set(SERVER_SOURCES
    src/lib.cpp
    src/signal.cpp
)

# Add subdirectories
add_subdirectory(src)

# Create netd_server static library
add_library(netd_server STATIC ${SERVER_SOURCES})

# Set include directories
target_include_directories(netd_server PRIVATE ${GLOBAL_INCLUDE_DIRS})

# Set compile definitions
target_compile_definitions(netd_server PRIVATE
    YANG_DIR="${YANG_DIR}"
)

# Link server sub-libraries
target_link_libraries(netd_server PRIVATE
    netd_server_netconf
    netd_server_store
)
