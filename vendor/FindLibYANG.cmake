# Custom FindLibYANG.cmake to use vendored libyang
# This overrides the system find_package behavior

include(FindPackageHandleStandardArgs)

# Force use of our vendored libyang (built in the libyang subdirectory)
set(LIBYANG_INCLUDE_DIR "${CMAKE_BINARY_DIR}/vendor_install/include")
set(LIBYANG_LIBRARY "${CMAKE_CURRENT_BINARY_DIR}/libyang/libyang.a")

# Set the variables that libnetconf2 expects
set(LIBYANG_INCLUDE_DIRS ${LIBYANG_INCLUDE_DIR})
set(LIBYANG_LIBRARIES ${LIBYANG_LIBRARY})

# Set version (we'll get this from the actual libyang build)
set(LIBYANG_VERSION "3.13.5")

# Mark as found
set(LIBYANG_FOUND TRUE)

# Mark as advanced so they don't show up in cmake-gui
mark_as_advanced(LIBYANG_INCLUDE_DIRS LIBYANG_LIBRARIES LIBYANG_VERSION)
