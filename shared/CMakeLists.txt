# Shared src sources
set(SHARED_SOURCES
    src/address.cpp
    src/exception.cpp
    src/logger.cpp
    src/route.cpp
    src/vrf.cpp
    src/yang.cpp
)

# Add subdirectories
add_subdirectory(src)

# Create the netd_shared static library
add_library(netd_shared STATIC)

# Add sources from src directory
target_sources(netd_shared PRIVATE ${SHARED_SOURCES})

# Set include directories
target_include_directories(netd_shared PRIVATE ${GLOBAL_INCLUDE_DIRS})

# Set compile definitions
target_compile_definitions(netd_shared PRIVATE
    YANG_DIR="${YANG_DIR}"
)

# Link sub-libraries
target_link_libraries(netd_shared PRIVATE
    netd_shared_interface
    netd_shared_marshalling
    netd_shared_request
    netd_shared_response
)
