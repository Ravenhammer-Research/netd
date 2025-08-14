# Vendor libraries configuration
# Build vendored libraries in the correct order with proper isolation

# Force static libraries
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build shared libraries" FORCE)

# Set C standard to C11 for libyang compatibility
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Disable conflicting features for libyang
set(ENABLE_DOCS OFF CACHE BOOL "Build documentation" FORCE)
set(ENABLE_COVERAGE OFF CACHE BOOL "Enable coverage" FORCE)
set(ENABLE_ABICHECK OFF CACHE BOOL "Enable ABI check" FORCE)
set(ENABLE_SOURCEFORMAT OFF CACHE BOOL "Enable source format" FORCE)
set(ENABLE_PCRE2 OFF CACHE BOOL "Enable PCRE2 support" FORCE)

# Disable all optional features for libnetconf2
set(ENABLE_SSH_TLS OFF CACHE BOOL "Enable NETCONF over SSH and TLS support" FORCE)
set(ENABLE_DNSSEC OFF CACHE BOOL "Enable DNSSEC support" FORCE)
set(ENABLE_TESTS OFF CACHE BOOL "Build tests" FORCE)
set(ENABLE_VALGRIND_TESTS OFF CACHE BOOL "Build tests with valgrind" FORCE)
set(ENABLE_EXAMPLES OFF CACHE BOOL "Build examples" FORCE)
set(ENABLE_COMMON_TARGETS OFF CACHE BOOL "Define common custom target names" FORCE)

# Set install directory to build directory to avoid system installation conflicts
set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/vendor_install CACHE PATH "Installation Directory" FORCE)

# Create vendor install directories
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/vendor_install/lib)
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/vendor_install/include/libyang)
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/vendor_install/include/libnetconf2)
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/vendor_install/lib/pkgconfig)

# Copy libyang headers to the correct location with libyang subdirectory
file(GLOB LIBYANG_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/libyang/src/*.h")
foreach(header ${LIBYANG_HEADERS})
    get_filename_component(header_name ${header} NAME)
    file(COPY ${header} DESTINATION "${CMAKE_BINARY_DIR}/vendor_install/include/libyang")
endforeach()

# Copy plugin headers
file(GLOB LIBYANG_PLUGIN_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/libyang/src/plugins_exts/*.h")
foreach(header ${LIBYANG_PLUGIN_HEADERS})
    get_filename_component(header_name ${header} NAME)
    file(COPY ${header} DESTINATION "${CMAKE_BINARY_DIR}/vendor_install/include/libyang")
endforeach()

# Copy plugin type headers
file(GLOB LIBYANG_PLUGIN_TYPE_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/libyang/src/plugins_types/*.h")
foreach(header ${LIBYANG_PLUGIN_TYPE_HEADERS})
    get_filename_component(header_name ${header} NAME)
    file(COPY ${header} DESTINATION "${CMAKE_BINARY_DIR}/vendor_install/include/libyang")
endforeach()

# Add our custom FindLibYANG.cmake to the module path so libnetconf2 uses it
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}")

# Set PKG_CONFIG_PATH to use our vendored libyang and system pkg-config
set(ENV{PKG_CONFIG_PATH} "${CMAKE_BINARY_DIR}/vendor_install/lib/pkgconfig:/usr/local/libdata/pkgconfig:$ENV{PKG_CONFIG_PATH}")

# Build libyang first
add_subdirectory(libyang)

# Build libnetconf2 (will use our custom FindLibYANG.cmake)
add_subdirectory(libnetconf2)

# Ensure libnetconf2 links against our vendored libyang
if(TARGET netconf2 AND TARGET yang)
    target_link_libraries(netconf2 yang)
    target_include_directories(netconf2 PRIVATE ${CMAKE_BINARY_DIR}/vendor_install/include)
    target_include_directories(netconf2 PRIVATE /usr/local/include)
    
    # Create a custom command that only copies if file doesn't exist or has changed
    add_custom_command(
        OUTPUT "${CMAKE_BINARY_DIR}/vendor_install/include/libyang/ly_config.h"
        COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_BINARY_DIR}/vendor/libyang/libyang/ly_config.h" "${CMAKE_BINARY_DIR}/vendor_install/include/libyang/ly_config.h"
        DEPENDS yang
        COMMENT "Copying ly_config.h to vendor install directory"
    )
    
    # Create a custom target to copy libnetconf2 headers and library
    add_custom_target(copy_netconf2_files
        COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_BINARY_DIR}/vendor/libnetconf2/include/nc_client.h" "${CMAKE_BINARY_DIR}/vendor_install/include/libnetconf2/nc_client.h"
        COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_BINARY_DIR}/vendor/libnetconf2/include/nc_server.h" "${CMAKE_BINARY_DIR}/vendor_install/include/libnetconf2/nc_server.h"
        COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_BINARY_DIR}/vendor/libnetconf2/include/nc_version.h" "${CMAKE_BINARY_DIR}/vendor_install/include/libnetconf2/nc_version.h"
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_BINARY_DIR}/vendor/libnetconf2/include/libnetconf2" "${CMAKE_BINARY_DIR}/vendor_install/include/libnetconf2"
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:netconf2> "${CMAKE_BINARY_DIR}/vendor_install/lib/libnetconf2.a"
        COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_BINARY_DIR}/vendor/libnetconf2/libnetconf2.pc" "${CMAKE_BINARY_DIR}/vendor_install/lib/pkgconfig/libnetconf2.pc"
        DEPENDS netconf2
        COMMENT "Copying libnetconf2 headers and library to vendor install directory"
    )
    
    # Create a custom target to copy libyang library and pkg-config
    add_custom_target(copy_libyang_files
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:yang> "${CMAKE_BINARY_DIR}/vendor_install/lib/libyang.a"
        COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_BINARY_DIR}/vendor/libyang/libyang.pc" "${CMAKE_BINARY_DIR}/vendor_install/lib/pkgconfig/libyang.pc"
        DEPENDS yang
        COMMENT "Copying libyang library and pkg-config to vendor install directory"
    )
    
    # Create a target that builds all copy targets
    add_custom_target(vendor_install ALL
        DEPENDS "${CMAKE_BINARY_DIR}/vendor_install/include/libyang/ly_config.h" copy_netconf2_files copy_libyang_files
        COMMENT "Installing vendored libraries to vendor_install directory"
    )
    
    # Make copy_netconf2_files depend on netconf2
    add_dependencies(copy_netconf2_files netconf2)
    
    # Make copy_libyang_files depend on yang
    add_dependencies(copy_libyang_files yang)
    

endif()
