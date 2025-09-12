# NetD Extensions Build System
# This CMakeLists.txt can be used to build extension modules

# Find the NetD shared library
find_library(NETD_SHARED_LIBRARY netd_shared
    PATHS ${CMAKE_INSTALL_PREFIX}/lib/netd
    NO_DEFAULT_PATH
)

if(NOT NETD_SHARED_LIBRARY)
    message(FATAL_ERROR "NetD shared library not found. Please install NetD first.")
endif()

# Example extension
add_library(example_extension SHARED example_extension.cpp)
target_link_libraries(example_extension ${NETD_SHARED_LIBRARY})
target_include_directories(example_extension PRIVATE ${CMAKE_INSTALL_PREFIX}/include)

# Set the output directory
set_target_properties(example_extension PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/extensions
    PREFIX ""
)

# Install the extension
install(TARGETS example_extension
    LIBRARY DESTINATION lib/netd/extensions
    COMPONENT extensions
)
