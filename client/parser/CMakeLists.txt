# Client parser CMakeLists.txt

# For now, we'll use placeholder parser files
# TODO: Implement proper Yacc/Lex processing

# Create placeholder parser files
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/get.c "// Placeholder for get.c\n")
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/set.c "// Placeholder for set.c\n")
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/commit.c "// Placeholder for commit.c\n")
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/delete.c "// Placeholder for delete.c\n")

# Add generated files to target sources
target_sources(netd_client PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}/get.c
    ${CMAKE_CURRENT_BINARY_DIR}/set.c
    ${CMAKE_CURRENT_BINARY_DIR}/commit.c
    ${CMAKE_CURRENT_BINARY_DIR}/delete.c
)

# Set include directories to find generated headers
target_include_directories(netd_client PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
