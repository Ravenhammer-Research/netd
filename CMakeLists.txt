cmake_minimum_required(VERSION 3.16)
project(NETD VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Set build type if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Add custom CMake modules
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Set PKG_CONFIG_PATH to include /usr/local/lib/pkgconfig
set(ENV{PKG_CONFIG_PATH} "/usr/local/lib/pkgconfig:$ENV{PKG_CONFIG_PATH}")

# Find required packages
find_package(PkgConfig REQUIRED)

# Find libyang
pkg_check_modules(LIBYANG REQUIRED libyang)
if(NOT LIBYANG_FOUND)
    message(FATAL_ERROR "libyang not found. Please install libyang development package.")
endif()

# Find libnetconf2
pkg_check_modules(LIBNETCONF2 REQUIRED libnetconf2)
if(NOT LIBNETCONF2_FOUND)
    message(FATAL_ERROR "libnetconf2 not found. Please install libnetconf2 development package.")
endif()

# Print pkg-config variables for debugging
message(STATUS "LIBYANG_LIBRARIES: ${LIBYANG_LIBRARIES}")
message(STATUS "LIBYANG_INCLUDE_DIRS: ${LIBYANG_INCLUDE_DIRS}")
message(STATUS "LIBYANG_LIBRARY_DIRS: ${LIBYANG_LIBRARY_DIRS}")
message(STATUS "LIBNETCONF2_LIBRARIES: ${LIBNETCONF2_LIBRARIES}")
message(STATUS "LIBNETCONF2_INCLUDE_DIRS: ${LIBNETCONF2_INCLUDE_DIRS}")
message(STATUS "LIBNETCONF2_LIBRARY_DIRS: ${LIBNETCONF2_LIBRARY_DIRS}")

# Find curses
find_package(Curses REQUIRED)

# Add subdirectories
add_subdirectory(shared)
add_subdirectory(server)
add_subdirectory(client)

# Set global compile options
add_compile_options(
    -Wall
    -Wextra
    -Wpedantic
    -Werror=return-type
    -Werror=uninitialized
    -Werror=format
    -Werror=format-security
    -Werror=array-bounds
    -Werror=non-virtual-dtor
    -Werror=delete-non-virtual-dtor
    -Werror=unused-parameter
    -Werror=unused-variable
    -Werror=unused-function
    -Werror=shadow
    -Werror=conversion
    -Werror=sign-conversion
    -Werror=float-conversion
    -Werror=double-promotion
    -fPIC
    -fstack-protector-strong
    -D_FORTIFY_SOURCE=2
)

# Global include directories are set per-target
