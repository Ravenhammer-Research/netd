# Candidate store sources
set(CANDIDATE_STORE_SOURCES
    base.cpp
    commit.cpp
)

# Create netd_server_store_candidate static library
add_library(netd_server_store_candidate STATIC ${CANDIDATE_STORE_SOURCES})

# Set include directories
target_include_directories(netd_server_store_candidate PRIVATE ${GLOBAL_INCLUDE_DIRS})

# Set compile definitions
target_compile_definitions(netd_server_store_candidate PRIVATE
    YANG_DIR="${YANG_DIR}"
)
