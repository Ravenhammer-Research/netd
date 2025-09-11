cmake_minimum_required(VERSION 3.16)

# Require Clang 18+ (for C++23 support)
find_program(CMAKE_CXX_COMPILER clang++)
if(NOT CMAKE_CXX_COMPILER)
  message(FATAL_ERROR "Clang C++ compiler not found. Please install clang++ 18 or later")
endif()

find_program(CMAKE_C_COMPILER clang)
if(NOT CMAKE_C_COMPILER)
  message(FATAL_ERROR "Clang C compiler not found. Please install clang 18 or later")
endif()

project(netd)

# Check Clang version (must be 18+)
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "18.0")
    message(FATAL_ERROR "Clang version ${CMAKE_CXX_COMPILER_VERSION} is too old. Please install Clang 18.0 or later")
  endif()
  message(STATUS "Clang version: ${CMAKE_CXX_COMPILER_VERSION} ✓")
else()
  message(FATAL_ERROR "Clang compiler required, but found: ${CMAKE_CXX_COMPILER_ID}")
endif()

# Set C++ standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

message(STATUS "=== NETD CMake Configuration ===")
message(STATUS "CMAKE_SOURCE_DIR: ${CMAKE_SOURCE_DIR}")
message(STATUS "CMAKE_BINARY_DIR: ${CMAKE_BINARY_DIR}")
message(STATUS "CMAKE_CXX_COMPILER: ${CMAKE_CXX_COMPILER}")
message(STATUS "CMAKE_CXX_STANDARD: ${CMAKE_CXX_STANDARD}")

# Set compile flags for hardening
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g3 -Wall -Wextra -Werror")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g3 -Wall -Wextra -Werror")

# Additional hardening flags (Clang-compatible)
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
message(STATUS "CMAKE_LIBRARY_PATH: ${CMAKE_LIBRARY_PATH}")

# Find required packages
find_package(PkgConfig REQUIRED)
set(ENV{PKG_CONFIG_PATH} "/usr/local/lib/pkgconfig/")
message(STATUS "PKG_CONFIG_PATH: $ENV{PKG_CONFIG_PATH}")

pkg_check_modules(LIBYANG REQUIRED libyang)

# Check for LLDP support
find_path(LLDP_INCLUDE_DIR lldpctl.h PATHS /usr/include /usr/local/include)
find_library(LLDP_LIBRARY lldpctl PATHS /usr/lib /usr/local/lib)
if(LLDP_INCLUDE_DIR AND LLDP_LIBRARY)
    set(LLDP_FOUND TRUE)
    message(STATUS "LLDP support found: ${LLDP_INCLUDE_DIR}, ${LLDP_LIBRARY}")
    add_compile_definitions(HAVE_LLDP=1)
else()
    set(LLDP_FOUND FALSE)
    message(STATUS "LLDP support not found - LLDP features will be disabled")
endif()

# Define global include directories
set(GLOBAL_INCLUDE_DIRS
    ${CMAKE_SOURCE_DIR}
    ${LIBYANG_INCLUDE_DIRS}
)

# Add LLDP include directory if found
if(LLDP_FOUND)
    list(APPEND GLOBAL_INCLUDE_DIRS ${LLDP_INCLUDE_DIR})
endif()

# Define YANG directory
set(YANG_DIR "${CMAKE_SOURCE_DIR}/yang/")
message(STATUS "YANG_DIR: ${YANG_DIR}")
message(STATUS "GLOBAL_INCLUDE_DIRS: ${GLOBAL_INCLUDE_DIRS}")

# Add subdirectories
add_subdirectory(shared)
add_subdirectory(server)
add_subdirectory(client)
add_subdirectory(freebsd)

# Create netd server executable
add_executable(netd server/src/main.cpp)

# Set include directories
target_include_directories(netd PRIVATE ${GLOBAL_INCLUDE_DIRS})

# Set compile definitions for server
target_compile_definitions(netd PRIVATE
    YANG_DIR="${YANG_DIR}"
)

# Add LLDP-specific definitions if LLDP is found
if(LLDP_FOUND)
    target_compile_definitions(netd PRIVATE NETD_OUI="00:1b:21")
endif()

# Link libraries
message(STATUS "LIBYANG_LIBRARIES: ${LIBYANG_LIBRARIES}")
set(NETD_LIBRARIES
    ${LIBYANG_LIBRARIES}
    netd_shared
    netd_server
    netd_freebsd
    pthread
    execinfo
    expat
)

# Add LLDP library if found
if(LLDP_FOUND)
    list(APPEND NETD_LIBRARIES ${LLDP_LIBRARY})
endif()

target_link_libraries(netd PRIVATE ${NETD_LIBRARIES})

# Create netd client executable
add_executable(netc client/src/main.cpp)

# Set include directories
target_include_directories(netc PRIVATE ${GLOBAL_INCLUDE_DIRS})

# Set compile definitions
target_compile_definitions(netc PRIVATE
    YANG_DIR="${YANG_DIR}"
)

# Link libraries
set(NETC_LIBRARIES
    ${LIBYANG_LIBRARIES}
    netd_shared
    netd_client
    netd_client_netconf
    curses
    pthread
    execinfo
    expat
)

# Add LLDP library if found
if(LLDP_FOUND)
    list(APPEND NETC_LIBRARIES ${LLDP_LIBRARY})
endif()

target_link_libraries(netc PRIVATE ${NETC_LIBRARIES})

message(STATUS "=== NETD CMake Configuration Complete ===")
