# Shared src sources
set(SHARED_SOURCES
    src/address.cpp
    src/exception.cpp
    src/logger.cpp
    src/route.cpp
    src/vrf.cpp
    src/yang.cpp
    src/backend.cpp
    src/transport.cpp
    src/tls.cpp
    src/dtls.cpp
    src/unix.cpp
    src/http.cpp
    src/quic.cpp
)

# LLDP sources are now handled in src/lldp/ subdirectory

# Add SCTP transport only on FreeBSD
if(CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")
    list(APPEND SHARED_SOURCES src/sctp.cpp)
    message(STATUS "SCTP transport enabled for FreeBSD")
else()
    message(STATUS "SCTP transport disabled (not FreeBSD)")
endif()

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
    netd_shared_netconf
    netd_shared_request
    netd_shared_response
)

# Add LLDP library if found
if(LLDP_FOUND)
    target_link_libraries(netd_shared PRIVATE netd_shared_lldp)
endif()
