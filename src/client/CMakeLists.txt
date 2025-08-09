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

# Main client source files
set(CLIENT_SRCS
    main.c
    client.c
    debug.c
)

# Add subdirectories
add_subdirectory(parser)
add_subdirectory(xml)
add_subdirectory(table)
add_subdirectory(netconf)

# Create the main executable
add_executable(net ${CLIENT_SRCS})

# Link with subdirectory libraries
target_link_libraries(net
    parser_lib
    xml_lib
    table_lib
    netconf_lib
    ${YANG_LIBRARIES}
    expat
    readline
    m
)

# Set include directories for this target
target_include_directories(net PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/parser
    ${CMAKE_CURRENT_SOURCE_DIR}/xml
    ${CMAKE_CURRENT_SOURCE_DIR}/table
    ${CMAKE_CURRENT_SOURCE_DIR}/netconf
)

# Set compiler flags for this target
target_compile_options(net PRIVATE
    -Wall -Wextra -Werror
) 