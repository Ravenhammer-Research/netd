# Response sources
set(RESPONSE_SOURCES
    base.cpp
    close.cpp
    commit.cpp
    copy.cpp
    delete.cpp
    discard.cpp
    edit.cpp
    hello.cpp
    kill.cpp
    lock.cpp
    unlock.cpp
    validate.cpp
)

# Add get subdirectory
add_subdirectory(get)

# Create netd_shared_response static library
add_library(netd_shared_response STATIC ${RESPONSE_SOURCES})

# Set include directories
target_include_directories(netd_shared_response PRIVATE ${GLOBAL_INCLUDE_DIRS})

# Set compile definitions
target_compile_definitions(netd_shared_response PRIVATE
    YANG_DIR="${YANG_DIR}"
)

# Link sub-libraries
target_link_libraries(netd_shared_response PRIVATE
    netd_shared_response_get
)
