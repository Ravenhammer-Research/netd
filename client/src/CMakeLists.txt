# Create netd_client static library
add_library(netd_client STATIC
    parser.cpp
    processor.cpp
    terminal.cpp
    lldp.cpp
)

# Set include directories
target_include_directories(netd_client PRIVATE ${GLOBAL_INCLUDE_DIRS})

# Set compile definitions
target_compile_definitions(netd_client PRIVATE
    YANG_DIR="${YANG_DIR}"
)

# Add subdirectories
add_subdirectory(netconf)
add_subdirectory(tui)

# Link libraries
target_link_libraries(netd_client PRIVATE
    netd_shared_netconf
    netd_client_parser
    netd_client_netconf
    netd_tui
)
