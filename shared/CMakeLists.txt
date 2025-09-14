# Shared src sources
set(SHARED_SOURCES
    src/address.cpp
    src/exception.cpp
    src/extension.cpp
    src/logger.cpp
    src/route.cpp
    src/vrf.cpp
    src/yang.cpp
    src/backend.cpp
    src/transport.cpp
    src/unix.cpp
    src/http.cpp
    src/quic.cpp
    src/stream.cpp
    src/socket.cpp
)

# Add TLS/DTLS sources only if OpenSSL is found
if(OPENSSL_FOUND)
    list(APPEND SHARED_SOURCES src/tls.cpp src/dtls.cpp)
    message(STATUS "TLS/DTLS support enabled (OpenSSL found)")
else()
    message(STATUS "TLS/DTLS support disabled (OpenSSL not found)")
endif()

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

# Set compile definitions for shared
target_compile_definitions(netd_shared PRIVATE
    YANG_DIR="${YANG_DIR}"
    YANG_DEV_DIR="${YANG_DEV_DIR}"
    EXTENSION_DIR="${EXTENSION_DIR}"
    EXTENSION_DEV_DIR="${EXTENSION_DEV_DIR}"
    BUILD_ID="${BUILD_ID}"
    GIT_REVISION="${GIT_REVISION}"
    GIT_REVISION_LONG="${GIT_REVISION_LONG}"
    BUILD_TYPE="${CMAKE_BUILD_TYPE}"
    PROJECT_VERSION="${PROJECT_VERSION}"
)

# Add DEBUG_BUILD macro if build type is DEBUG
if(CMAKE_BUILD_TYPE STREQUAL "DEBUG")
    target_compile_definitions(netd_shared PRIVATE DEBUG_BUILD=1)
endif()

# Add LLDP-specific definitions if LLDP is found
if(LLDP_FOUND)
    target_compile_definitions(netd_shared PRIVATE NETD_OUI="${NETD_OUI}")
endif()

# Set include directories
target_include_directories(netd_shared PRIVATE ${GLOBAL_INCLUDE_DIRS})

# Link sub-libraries
target_link_libraries(netd_shared PRIVATE
    netd_shared_interface
    netd_shared_marshalling
    netd_shared_netconf
    netd_shared_request
    netd_shared_response
    netd_shared_xml
)

# Add LLDP library if found
if(LLDP_FOUND)
    target_link_libraries(netd_shared PRIVATE netd_shared_lldp)
endif()

# Set pkg-config variables
set(OPENSSL_REQUIRES "")
set(LLDP_REQUIRES "")

if(OPENSSL_FOUND)
    set(OPENSSL_REQUIRES "openssl")
endif()

if(LLDP_FOUND)
    set(LLDP_REQUIRES "liblldpctl")
endif()

# Generate pkg-config file
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/netd-shared.pc.in"
    "${CMAKE_CURRENT_BINARY_DIR}/netd-shared.pc"
    @ONLY
)

# Install pkg-config file
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/netd-shared.pc"
    DESTINATION lib/pkgconfig
    COMPONENT shared
)
