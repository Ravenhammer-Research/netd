# Copyright (c) 2025 Paige Thompson / Raven Hammer Research (paige@paige.bio)
# All rights reserved.
#
# Redistribution and use in source code and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice, this
#    list of conditions and the following disclaimer.
#
# 2. Redistributions in binary form must reproduce the above copyright notice,
#    this list of conditions and the following disclaimer in the documentation
#    and/or other materials provided with the distribution.
#
# 3. Neither the name of the copyright holder nor the names of its
#    contributors may be used to endorse or promote products derived from
#    this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

cmake_minimum_required(VERSION 3.16)
project(net VERSION 1.0.0 LANGUAGES C)

# Set C standard
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Compiler flags
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Werror")
set(CMAKE_C_FLAGS_DEBUG "-g -O0")
set(CMAKE_C_FLAGS_RELEASE "-O2 -DNDEBUG")

# Find required packages
find_package(PkgConfig REQUIRED)

# Set libyang paths directly since pkg-config is unreliable
set(YANG_INCLUDE_DIRS "/usr/local/include")
# Set netconf library
set(NETCONF_INCLUDE_DIRS "/usr/local/include")

# Add library search paths
link_directories(/usr/local/lib /usr/lib)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/src/shared)
include_directories(${YANG_INCLUDE_DIRS})
include_directories(${NETCONF_INCLUDE_DIRS})

# Define constants that can be used by targets
set(NETD_DEFINES
    MAX_VRF_NAME_LEN=64
    MAX_IFNAME_LEN=15
    MAX_GROUP_NAME_LEN=15
    MAX_GROUPS_PER_IF=65536
    MAX_BRIDGE_MEMBERS=65536
    MAX_LAGG_MEMBERS=65536
    MAX_VRFS=65536
    MAX_IPV4_ADDRESSES_PER_IF=256
    MAX_IPV6_ADDRESSES_PER_IF=256
    MAX_NETCONF_SESSIONS=64
    NETD_SOCKET_PATH="/var/run/netd.sock"
    YANG_GLOBAL_PATH="/usr/local/share/yang/modules/libyang/"
    YANG_LOCAL_PATH="./yang"
)

# Add subdirectories
add_subdirectory(src/shared)
#add_subdirectory(src/client)
add_subdirectory(src/server)

# Install targets
install(TARGETS netd
    RUNTIME DESTINATION bin
)

# Install YANG files
install(DIRECTORY yang/
    DESTINATION share/netd
    FILES_MATCHING PATTERN "*.yang"
)

# Install configuration directory
install(DIRECTORY DESTINATION etc/netd) 