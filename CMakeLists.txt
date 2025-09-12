cmake_minimum_required(VERSION 3.16)

# Prefer Clang over GCC, but allow GCC as fallback
find_program(CLANG_CXX_COMPILER clang++)
find_program(CLANG_C_COMPILER clang)
find_program(GCC_CXX_COMPILER g++)
find_program(GCC_C_COMPILER gcc)

# Set compiler preference: Clang first, then GCC
if(CLANG_CXX_COMPILER AND CLANG_C_COMPILER)
    set(CMAKE_CXX_COMPILER ${CLANG_CXX_COMPILER})
    set(CMAKE_C_COMPILER ${CLANG_C_COMPILER})
    message(STATUS "Using Clang compiler (preferred)")
elseif(GCC_CXX_COMPILER AND GCC_C_COMPILER)
    set(CMAKE_CXX_COMPILER ${GCC_CXX_COMPILER})
    set(CMAKE_C_COMPILER ${GCC_C_COMPILER})
    message(STATUS "Using GCC compiler (Clang not found)")
else()
    message(FATAL_ERROR "No suitable C++ compiler found. Please install Clang 18+ or GCC 11+")
endif()

project(netd VERSION 1.0.0)

# Get git revision for build ID
include(cmake/GetGitRevisionDescription.cmake)
get_git_revision_description(GIT_REVISION)
get_git_revision_description_long(GIT_REVISION_LONG)

# Set build ID
if(GIT_REVISION STREQUAL "unknown")
    set(BUILD_ID "unknown")
else()
    # Check if working directory is dirty
    execute_process(
        COMMAND git diff-index --quiet HEAD --
        RESULT_VARIABLE GIT_DIRTY_RESULT
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_QUIET
        ERROR_QUIET
    )
    
    if(GIT_DIRTY_RESULT EQUAL 0)
        # Not dirty - use branch-timestamp format
        execute_process(
            COMMAND git rev-parse --abbrev-ref HEAD
            OUTPUT_VARIABLE GIT_BRANCH
            OUTPUT_STRIP_TRAILING_WHITESPACE
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        )
        string(TIMESTAMP BUILD_TIMESTAMP "%Y%m%d%H%M%S")
        set(BUILD_ID "${GIT_BRANCH}-${BUILD_TIMESTAMP}")
    else()
        # Dirty - use hash-dirty format
        set(BUILD_ID "${GIT_REVISION}-dirty")
    endif()
endif()


# Check compiler version and capabilities
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "18.0")
        message(FATAL_ERROR "Clang version ${CMAKE_CXX_COMPILER_VERSION} is too old. Please install Clang 18.0 or later")
    endif()
    message(STATUS "Clang version: ${CMAKE_CXX_COMPILER_VERSION}")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "11.0")
        message(FATAL_ERROR "GCC version ${CMAKE_CXX_COMPILER_VERSION} is too old. Please install GCC 11.0 or later")
    endif()
    message(STATUS "GCC version: ${CMAKE_CXX_COMPILER_VERSION}")
else()
    message(FATAL_ERROR "Unsupported compiler: ${CMAKE_CXX_COMPILER_ID}")
endif()

# Set C++ standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set up build type validation
include(cmake/GetBuildType.cmake)

# Set compile flags for hardening
if(CMAKE_BUILD_TYPE STREQUAL "DEBUG")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g3 -Wall -Wextra -Werror")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g3 -Wall -Wextra -Werror")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror -O2")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Werror -O2")
    # Strip symbols in RELEASE builds
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -s")
endif()

# Additional hardening flags (common to both Clang and GCC)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fstack-protector-strong -fPIE -Wformat -Wformat-security")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fstack-protector-strong -fPIE -Wformat -Wformat-security")

# Add _FORTIFY_SOURCE only for GCC
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_FORTIFY_SOURCE=2")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_FORTIFY_SOURCE=2")
endif()

# Link hardening flags
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-z,relro -Wl,-z,now -pie")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,-z,relro -Wl,-z,now")

# Set library search paths
set(CMAKE_LIBRARY_PATH "/usr/lib" "/usr/local/lib")
link_directories("/usr/lib" "/usr/local/lib")

# Find required packages
find_package(PkgConfig REQUIRED)
set(ENV{PKG_CONFIG_PATH} "/usr/local/lib/pkgconfig/")

pkg_check_modules(LIBYANG REQUIRED libyang)

# Check for ncurses support (for client)
pkg_check_modules(NCURSES REQUIRED ncurses)

# Check for OpenSSL support
pkg_check_modules(OPENSSL REQUIRED openssl)
if(OPENSSL_FOUND)
    message(STATUS "OpenSSL support found: ${OPENSSL_VERSION}")
    add_compile_definitions(HAVE_OPENSSL=1)
else()
    message(FATAL_ERROR "OpenSSL not found - required for TLS/DTLS support")
endif()

# Check for LLDP support
find_path(LLDP_INCLUDE_DIR lldpctl.h PATHS /usr/include /usr/local/include)
find_library(LLDP_LIBRARY lldpctl PATHS /usr/lib /usr/local/lib)
if(LLDP_INCLUDE_DIR AND LLDP_LIBRARY)
    set(LLDP_FOUND TRUE)
    message(STATUS "LLDP support found: ${LLDP_INCLUDE_DIR}, ${LLDP_LIBRARY}")
    add_compile_definitions(HAVE_LLDP=1)
    set(NETD_OUI "00:1b:21")
else()
    set(LLDP_FOUND FALSE)
    message(STATUS "LLDP support not found - LLDP features will be disabled")
endif()

# Check for execinfo library (for stack traces)
find_library(EXECINFO_LIBRARY execinfo)
if(EXECINFO_LIBRARY)
    set(EXECINFO_FOUND TRUE)
    message(STATUS "execinfo support found: ${EXECINFO_LIBRARY}")
    add_compile_definitions(HAVE_EXECINFO=1)
else()
    set(EXECINFO_FOUND FALSE)
    message(STATUS "execinfo support not found - stack traces disabled")
endif()

# Check for pthread library
find_library(PTHREAD_LIBRARY pthread)
if(PTHREAD_LIBRARY)
    set(PTHREAD_FOUND TRUE)
    message(STATUS "pthread support found: ${PTHREAD_LIBRARY}")
else()
    set(PTHREAD_FOUND FALSE)
    message(STATUS "pthread support not found - threading disabled")
endif()

# Define global include directories
set(GLOBAL_INCLUDE_DIRS
    ${CMAKE_SOURCE_DIR}
    ${LIBYANG_INCLUDE_DIRS}
    ${NCURSES_INCLUDE_DIRS}
    ${OPENSSL_INCLUDE_DIRS}
)

# Add LLDP include directory if found
if(LLDP_FOUND)
    list(APPEND GLOBAL_INCLUDE_DIRS ${LLDP_INCLUDE_DIR})
endif()

# Define YANG directories
set(YANG_DIR "${CMAKE_INSTALL_PREFIX}/share/netd/yang/")
set(YANG_DEV_DIR "${CMAKE_SOURCE_DIR}/yang/")

# Add subdirectories
add_subdirectory(shared)
add_subdirectory(server)
add_subdirectory(client)
add_subdirectory(freebsd)

# Include FreeBSD package generation (only on FreeBSD)
if(CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")
    include(cmake/FreeBSDPackage.cmake)
endif()

# Create netd server executable
add_executable(netd server/src/main.cpp)

# Set include directories
target_include_directories(netd PRIVATE ${GLOBAL_INCLUDE_DIRS})

message(STATUS "")
# Link libraries
message(STATUS "Build Settings:")
message(STATUS "  CMAKE_SOURCE_DIR: ${CMAKE_SOURCE_DIR}")
message(STATUS "  CMAKE_BINARY_DIR: ${CMAKE_BINARY_DIR}")
message(STATUS "  CMAKE_CXX_COMPILER: ${CMAKE_CXX_COMPILER}")
message(STATUS "  CMAKE_CXX_STANDARD: ${CMAKE_CXX_STANDARD}")
message(STATUS "  CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")
message(STATUS "  PROJECT_VERSION: ${PROJECT_VERSION}")
message(STATUS "  CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS}")
message(STATUS "  CMAKE_LIBRARY_PATH: ${CMAKE_LIBRARY_PATH}")
message(STATUS "  PKG_CONFIG_PATH: $ENV{PKG_CONFIG_PATH}")
message(STATUS "  GLOBAL_INCLUDE_DIRS: ${GLOBAL_INCLUDE_DIRS}")
message(STATUS "  YANG_DIR: ${YANG_DIR}")
message(STATUS "  YANG_DEV_DIR: ${YANG_DEV_DIR}")
message(STATUS "  GIT_REVISION: ${GIT_REVISION}")
message(STATUS "  GIT_REVISION_LONG: ${GIT_REVISION_LONG}")
message(STATUS "  BUILD_ID: ${BUILD_ID}")
message(STATUS "  LIBYANG_LIBRARIES: ${LIBYANG_LIBRARIES}")
message(STATUS "  NCURSES_LIBRARIES: ${NCURSES_LIBRARIES}")
message(STATUS "  OPENSSL_LIBRARIES: ${OPENSSL_LIBRARIES}")
if(LLDP_FOUND)
    message(STATUS "  LLDP_LIBRARY: ${LLDP_LIBRARY}")
    message(STATUS "  NETD_OUI: ${NETD_OUI}")
endif()
if(EXECINFO_FOUND)
    message(STATUS "  EXECINFO_LIBRARY: ${EXECINFO_LIBRARY}")
endif()
if(PTHREAD_FOUND)
    message(STATUS "  PTHREAD_LIBRARY: ${PTHREAD_LIBRARY}")
endif()
message(STATUS "")
set(NETD_LIBRARIES
    ${LIBYANG_LIBRARIES}
    ${OPENSSL_LIBRARIES}
    netd_shared
    netd_server
    netd_freebsd
)

# Add LLDP library if found
if(LLDP_FOUND)
    list(APPEND NETD_LIBRARIES ${LLDP_LIBRARY})
endif()

# Add execinfo library if found
if(EXECINFO_FOUND)
    list(APPEND NETD_LIBRARIES ${EXECINFO_LIBRARY})
endif()

# Add pthread library if found
if(PTHREAD_FOUND)
    list(APPEND NETD_LIBRARIES ${PTHREAD_LIBRARY})
endif()

target_link_libraries(netd PRIVATE ${NETD_LIBRARIES})

# Create netd client executable
add_executable(netc client/src/main.cpp)

# Set include directories
target_include_directories(netc PRIVATE ${GLOBAL_INCLUDE_DIRS})

# Set compile definitions
target_compile_definitions(netc PRIVATE
    YANG_DIR="${YANG_DEV_DIR}"
    BUILD_ID="${BUILD_ID}"
)

# Link libraries
set(NETC_LIBRARIES
    ${LIBYANG_LIBRARIES}
    ${NCURSES_LIBRARIES}
    ${OPENSSL_LIBRARIES}
    netd_shared
    netd_client
    netd_client_netconf
)

# Add LLDP library if found
if(LLDP_FOUND)
    list(APPEND NETC_LIBRARIES ${LLDP_LIBRARY})
endif()

# Add execinfo library if found
if(EXECINFO_FOUND)
    list(APPEND NETC_LIBRARIES ${EXECINFO_LIBRARY})
endif()

# Add pthread library if found
if(PTHREAD_FOUND)
    list(APPEND NETC_LIBRARIES ${PTHREAD_LIBRARY})
endif()

target_link_libraries(netc PRIVATE ${NETC_LIBRARIES})

# Set default install prefix
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "/usr/local" CACHE PATH "Installation directory" FORCE)
endif()

# Install executables
install(TARGETS netd
    RUNTIME DESTINATION sbin
    COMPONENT netd
)

install(TARGETS netc
    RUNTIME DESTINATION bin
    COMPONENT netc
)

# Install shared libraries
install(TARGETS netd_shared
    LIBRARY DESTINATION lib/netd
    ARCHIVE DESTINATION lib/netd
    COMPONENT shared
)

install(TARGETS netd_server
    LIBRARY DESTINATION lib/netd
    ARCHIVE DESTINATION lib/netd
    COMPONENT netd
)

install(TARGETS netd_client
    LIBRARY DESTINATION lib/netd
    ARCHIVE DESTINATION lib/netd
    COMPONENT netc
)

install(TARGETS netd_client_netconf
    LIBRARY DESTINATION lib/netd
    ARCHIVE DESTINATION lib/netd
    COMPONENT netc
)

install(TARGETS netd_freebsd
    LIBRARY DESTINATION lib/netd
    ARCHIVE DESTINATION lib/netd
    COMPONENT netd
)

# Install headers
install(DIRECTORY shared/include/
    DESTINATION include/netd
    COMPONENT shared
    FILES_MATCHING PATTERN "*.hpp"
)

install(DIRECTORY server/include/
    DESTINATION include/netd
    COMPONENT netd
    FILES_MATCHING PATTERN "*.hpp"
)

install(DIRECTORY client/include/
    DESTINATION include/netd
    COMPONENT netc
    FILES_MATCHING PATTERN "*.hpp"
)

install(DIRECTORY freebsd/include/
    DESTINATION include/netd
    COMPONENT netd
    FILES_MATCHING PATTERN "*.hpp"
)

# Install YANG files
install(DIRECTORY yang/
    DESTINATION share/netd/yang
    COMPONENT shared
    FILES_MATCHING 
    PATTERN "*.yang"
    PATTERN "*.txt"
    PATTERN "*.md"
    PATTERN "*.sh"
)

# Individual install targets
add_custom_target(install_netc
    COMMAND ${CMAKE_COMMAND} -E echo "Installing netc client"
    COMMAND ${CMAKE_COMMAND} -DCOMPONENT=netc -P cmake_install.cmake
    COMMENT "Installing netc client"
)

add_custom_target(install_shared
    COMMAND ${CMAKE_COMMAND} -E echo "Installing shared libraries and headers"
    COMMAND ${CMAKE_COMMAND} -DCOMPONENT=shared -P cmake_install.cmake
    COMMENT "Installing shared libraries and headers"
)

add_custom_target(install_netd
    COMMAND ${CMAKE_COMMAND} -E echo "Installing netd server"
    COMMAND ${CMAKE_COMMAND} -DCOMPONENT=netd -P cmake_install.cmake
    COMMENT "Installing netd server"
)

add_custom_target(install_yang
    COMMAND ${CMAKE_COMMAND} -E echo "Installing YANG files"
    COMMAND ${CMAKE_COMMAND} -DCOMPONENT=shared -P cmake_install.cmake
    COMMENT "Installing YANG files"
)

# Generate uninstall script for installation
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/cmake_uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
    IMMEDIATE @ONLY
)

# Install uninstall script
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
    DESTINATION share/cmake/netd
    RENAME uninstall.cmake
    COMPONENT shared
)

# Copy install manifest to install location after installation
install(CODE "
    if(EXISTS \"\${CMAKE_CURRENT_BINARY_DIR}/install_manifest.txt\")
        file(COPY \"\${CMAKE_CURRENT_BINARY_DIR}/install_manifest.txt\" 
             DESTINATION \"\${CMAKE_INSTALL_PREFIX}/share/cmake/netd/\")
    endif()
")

# Create uninstall target
if(NOT TARGET uninstall)
    add_custom_target(uninstall
        COMMAND ${CMAKE_COMMAND} -P ${CMAKE_INSTALL_PREFIX}/share/cmake/netd/uninstall.cmake
        COMMENT "Uninstalling netd"
    )
endif()

message(STATUS "Install prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "  Binaries: ${CMAKE_INSTALL_PREFIX}/sbin/netd, ${CMAKE_INSTALL_PREFIX}/bin/netc")
message(STATUS "  Libraries: ${CMAKE_INSTALL_PREFIX}/lib/netd/")
message(STATUS "  Headers: ${CMAKE_INSTALL_PREFIX}/include/netd/")
message(STATUS "  YANG files: ${CMAKE_INSTALL_PREFIX}/share/netd/yang/")
message(STATUS "  pkg-config: ${CMAKE_INSTALL_PREFIX}/lib/pkgconfig/netd-shared.pc")
message(STATUS "  Uninstaller: ${CMAKE_INSTALL_PREFIX}/share/cmake/netd/uninstall.cmake")
message(STATUS "")
message(STATUS "Install targets:")
message(STATUS "  make install          - Install everything")
message(STATUS "  make install_netc     - Install client only")
message(STATUS "  make install_shared   - Install shared libraries and headers")
message(STATUS "  make install_netd     - Install server only")
message(STATUS "  make install_yang     - Install YANG files only")
message(STATUS "  make uninstall        - Remove all installed files")
message(STATUS "")