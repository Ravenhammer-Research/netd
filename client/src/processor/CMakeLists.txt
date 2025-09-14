# Create netd_client_processor static library
add_library(netd_client_processor STATIC 
    parser.cpp
    command.cpp
    actions.cpp
    token.cpp
    completion.cpp
)

# Set include directories
target_include_directories(netd_client_processor PRIVATE 
    ${GLOBAL_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../parser
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

# Set target properties
target_include_directories(netd_client_processor PRIVATE 
    ${GLOBAL_INCLUDE_DIRS}
    ${CMAKE_BINARY_DIR}
)

# Set compile definitions
target_compile_definitions(netd_client_processor PRIVATE
    YANG_DIR="${YANG_DIR}"
)

# Suppress unused function warnings for generated lexer code
target_compile_options(netd_client_processor PRIVATE
    -Wno-unused-function
    -Wno-unneeded-internal-declaration
)

