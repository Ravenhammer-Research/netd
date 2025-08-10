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

# Find Flex and Yacc - use more flexible approach
find_program(FLEX_EXECUTABLE flex)
find_program(YACC_EXECUTABLE yacc)

if(NOT FLEX_EXECUTABLE)
    message(FATAL_ERROR "Flex not found. Please install flex package.")
endif()

if(NOT YACC_EXECUTABLE)
    message(FATAL_ERROR "Yacc not found. Please install yacc package.")
endif()

# Parser source files
set(PARSER_SRCS
    utils.c
)

# Generate parser and lexer files using custom commands
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/parser.tab.c
    COMMAND ${YACC_EXECUTABLE} -d -b ${CMAKE_CURRENT_BINARY_DIR}/parser ${CMAKE_CURRENT_SOURCE_DIR}/parser.y
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/parser.y
    COMMENT "Generating parser.tab.c from parser.y"
)

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/lexer.c
    COMMAND ${FLEX_EXECUTABLE} -o ${CMAKE_CURRENT_BINARY_DIR}/lexer.c ${CMAKE_CURRENT_SOURCE_DIR}/lexer.l
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/lexer.l ${CMAKE_CURRENT_BINARY_DIR}/parser.tab.h
    COMMENT "Generating lexer.c from lexer.l"
)

# Create parser library
add_library(parser_lib STATIC
    ${PARSER_SRCS}
    ${CMAKE_CURRENT_BINARY_DIR}/parser.tab.c
    ${CMAKE_CURRENT_BINARY_DIR}/lexer.c
)

# Set include directories for parser library
target_include_directories(parser_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_SOURCE_DIR}/../netconf
)

 