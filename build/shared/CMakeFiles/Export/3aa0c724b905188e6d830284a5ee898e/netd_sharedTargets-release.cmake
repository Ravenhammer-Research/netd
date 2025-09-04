#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "netd::netd_shared" for configuration "Release"
set_property(TARGET netd::netd_shared APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(netd::netd_shared PROPERTIES
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/libnetd_shared.so.1.0.0"
  IMPORTED_SONAME_RELEASE "libnetd_shared.so.1"
  )

list(APPEND _cmake_import_check_targets netd::netd_shared )
list(APPEND _cmake_import_check_files_for_netd::netd_shared "${_IMPORT_PREFIX}/lib/libnetd_shared.so.1.0.0" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
