# Include custom CMake modules
include(${CMAKE_SOURCE_DIR}/cmake/Lex.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/Yacc.cmake)

# Create netd_client_parser static library
add_library(netd_client_parser STATIC)

# Add lex sources
add_lex_source(netd_client_parser commit.l)
add_lex_source(netd_client_parser delete.l)
add_lex_source(netd_client_parser get.l)
add_lex_source(netd_client_parser set.l)

# Add yacc sources
add_yacc_source(netd_client_parser commit.y)
add_yacc_source(netd_client_parser delete.y)
add_yacc_source(netd_client_parser get.y)
add_yacc_source(netd_client_parser set.y)

# Set include directories
target_include_directories(netd_client_parser PRIVATE ${GLOBAL_INCLUDE_DIRS})

# Set compile definitions
target_compile_definitions(netd_client_parser PRIVATE
    YANG_DIR="${YANG_DIR}"
)
