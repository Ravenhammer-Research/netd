cmake_minimum_required(VERSION 3.16)
project(netd)

message(STATUS "=== NETD CMake Configuration ===")
message(STATUS "CMAKE_SOURCE_DIR: ${CMAKE_SOURCE_DIR}")
message(STATUS "CMAKE_BINARY_DIR: ${CMAKE_BINARY_DIR}")
message(STATUS "CMAKE_CXX_COMPILER: ${CMAKE_CXX_COMPILER}")
message(STATUS "CMAKE_CXX_STANDARD: ${CMAKE_CXX_STANDARD}")

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set library search paths
set(CMAKE_LIBRARY_PATH "/usr/lib" "/usr/local/lib")
link_directories("/usr/lib" "/usr/local/lib")
message(STATUS "CMAKE_LIBRARY_PATH: ${CMAKE_LIBRARY_PATH}")

# Find required packages
find_package(PkgConfig REQUIRED)
set(ENV{PKG_CONFIG_PATH} "/usr/local/lib/pkgconfig/")
message(STATUS "PKG_CONFIG_PATH: $ENV{PKG_CONFIG_PATH}")

pkg_check_modules(LIBYANG REQUIRED libyang)
pkg_check_modules(LIBNETCONF2 REQUIRED libnetconf2)

# Define global include directories
set(GLOBAL_INCLUDE_DIRS
    ${CMAKE_SOURCE_DIR}
    ${LIBYANG_INCLUDE_DIRS}
    ${LIBNETCONF2_INCLUDE_DIRS}
)

# Define YANG directory
set(YANG_DIR "${CMAKE_SOURCE_DIR}/yang")
set(YANG_DIR "${YANG_DIR}:${CMAKE_SOURCE_DIR}/yang/standard")
set(YANG_DIR "${YANG_DIR}:${CMAKE_SOURCE_DIR}/yang/experimental")
set(YANG_DIR "${YANG_DIR}:${CMAKE_SOURCE_DIR}/yang/vendor")
set(YANG_DIR "${YANG_DIR}:/usr/local/share/yang/modules/libnetconf2")
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

# Link libraries
message(STATUS "LIBYANG_LIBRARIES: ${LIBYANG_LIBRARIES}")
message(STATUS "LIBNETCONF2_LIBRARIES: ${LIBNETCONF2_LIBRARIES}")
target_link_libraries(netd PRIVATE
    ${LIBYANG_LIBRARIES}
    ${LIBNETCONF2_LIBRARIES}
    netd_shared
    netd_server
    netd_freebsd
    pthread
)

# Create netd client executable
add_executable(netc client/src/main.cpp)

# Set include directories
target_include_directories(netc PRIVATE ${GLOBAL_INCLUDE_DIRS})

# Set compile definitions
target_compile_definitions(netc PRIVATE
    YANG_DIR="${YANG_DIR}"
)

# Link libraries
target_link_libraries(netc PRIVATE
    ${LIBYANG_LIBRARIES}
    ${LIBNETCONF2_LIBRARIES}
    netd_shared
    netd_client
    curses
)

message(STATUS "=== NETD CMake Configuration Complete ===")
