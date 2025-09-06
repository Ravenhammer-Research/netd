# Base interface sources
set(BASE_INTERFACE_SOURCES
    ether.cpp
    master.cpp
    tunnel.cpp
)

# Create netd_shared_interface_base static library
add_library(netd_shared_interface_base STATIC ${BASE_INTERFACE_SOURCES})

# Set include directories
target_include_directories(netd_shared_interface_base PRIVATE ${GLOBAL_INCLUDE_DIRS})

# Set compile definitions
target_compile_definitions(netd_shared_interface_base PRIVATE
    YANG_DIR="${YANG_DIR}"
)
