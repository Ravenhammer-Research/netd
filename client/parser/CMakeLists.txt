# Client parser CMakeLists.txt

# Include custom Yacc and Lex processing modules
include(${CMAKE_SOURCE_DIR}/cmake/Yacc.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/Lex.cmake)

# Process Yacc files to generate parsers
add_yacc_source(netd_client get.y)
add_yacc_source(netd_client set.y)
add_yacc_source(netd_client commit.y)
add_yacc_source(netd_client delete.y)

# Process Lex files to generate lexers
add_lex_source(netd_client get.l)
add_lex_source(netd_client set.l)
add_lex_source(netd_client commit.l)
add_lex_source(netd_client delete.l)
