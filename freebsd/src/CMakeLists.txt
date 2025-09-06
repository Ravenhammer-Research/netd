# FreeBSD sources
set(FREEBSD_SOURCES
    route.cpp
    vrf.cpp
)

# Add subdirectories
add_subdirectory(interface)

# Create netd_freebsd static library
add_library(netd_freebsd STATIC ${FREEBSD_SOURCES})

# Set include directories
target_include_directories(netd_freebsd PRIVATE ${GLOBAL_INCLUDE_DIRS})

# Set compile definitions
target_compile_definitions(netd_freebsd PRIVATE
    YANG_DIR="${YANG_DIR}"
)

# Link libraries
target_link_libraries(netd_freebsd PRIVATE
    netd_freebsd_interface
)
