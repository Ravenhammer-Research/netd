# Include custom CMake modules
include(${CMAKE_SOURCE_DIR}/cmake/Lex.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/Yacc.cmake)

# Create netd_client_parser static library
add_library(netd_client_parser STATIC)

# Add unified lex and yacc sources
add_lex_source(netd_client_parser parser.l)
add_yacc_source(netd_client_parser parser.y)

# Set include directories
target_include_directories(netd_client_parser PRIVATE ${GLOBAL_INCLUDE_DIRS})

# Set compile definitions
target_compile_definitions(netd_client_parser PRIVATE
    YANG_DIR="${YANG_DIR}"
)

# Suppress unused function warnings for generated lexer code
target_compile_options(netd_client_parser PRIVATE
    -Wno-unused-function
    -Wno-unneeded-internal-declaration
)
