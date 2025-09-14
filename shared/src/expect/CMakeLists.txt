# Expect system source files
set(EXPECT_SOURCES
    base.cpp
    interface.cpp
    manager.cpp
)

# Create netd_shared_expect static library
add_library(netd_shared_expect STATIC ${EXPECT_SOURCES})

# Set include directories
target_include_directories(netd_shared_expect PRIVATE ${GLOBAL_INCLUDE_DIRS})

