# FreeBSDPackage.cmake
# Generate FreeBSD .pkg package

function(create_freebsd_package)
    # Check if we're on FreeBSD
    if(NOT CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")
        message(WARNING "FreeBSD package generation is only supported on FreeBSD systems")
        return()
    endif()

    # Check if pkg is available
    find_program(PKG_CMD pkg)
    if(NOT PKG_CMD)
        message(WARNING "pkg command not found - cannot generate FreeBSD package")
        return()
    endif()

    # Set package variables
    set(PACKAGE_NAME "netd")
    set(PACKAGE_VERSION "${PROJECT_VERSION}")
    set(PACKAGE_MAINTAINER "netd@example.com")
    set(PACKAGE_DESCRIPTION "NETD - Network management daemon and client")
    set(PACKAGE_WEBSITE "https://github.com/example/netd")
    set(PACKAGE_LICENSE "BSD3CLAUSE")

    # Create package directory structure
    set(PACKAGE_DIR "${CMAKE_BINARY_DIR}/freebsd-package")
    set(PACKAGE_ROOT "${PACKAGE_DIR}/root")
    set(PACKAGE_META "${PACKAGE_DIR}/meta")

    # Clean and create directories
    file(REMOVE_RECURSE ${PACKAGE_DIR})
    file(MAKE_DIRECTORY ${PACKAGE_ROOT})
    file(MAKE_DIRECTORY ${PACKAGE_META})

    # Create +MANIFEST file
    set(MANIFEST_CONTENT "+MANIFEST
name: ${PACKAGE_NAME}
version: ${PACKAGE_VERSION}
origin: net/netd
comment: ${PACKAGE_DESCRIPTION}
maintainer: ${PACKAGE_MAINTAINER}
www: ${PACKAGE_WEBSITE}
licenselogic: single
licenses: ${PACKAGE_LICENSE}
categories: net
prefix: ${CMAKE_INSTALL_PREFIX}
")

    # Add dependencies
    set(MANIFEST_CONTENT "${MANIFEST_CONTENT}deps:
")

    # Add libyang dependency
    set(MANIFEST_CONTENT "${MANIFEST_CONTENT}  libyang:
    origin: devel/libyang
    version: 2.0.0
")

    # Add OpenSSL dependency
    set(MANIFEST_CONTENT "${MANIFEST_CONTENT}  openssl:
    origin: security/openssl
    version: 3.0.0
")

    # Add ncurses dependency
    set(MANIFEST_CONTENT "${MANIFEST_CONTENT}  ncurses:
    origin: devel/ncurses
    version: 6.3
")

    # Add LLDP dependency if found
    if(LLDP_FOUND)
        set(MANIFEST_CONTENT "${MANIFEST_CONTENT}  lldpd:
    origin: net/lldpd
    version: 1.0.0
")
    endif()

    # Add files section
    set(MANIFEST_CONTENT "${MANIFEST_CONTENT}files:
")

    # Write manifest file
    file(WRITE "${PACKAGE_META}/+MANIFEST" "${MANIFEST_CONTENT}")

    # Create +DISPLAY file
    set(DISPLAY_CONTENT "NETD - Network Management Daemon

NETD is a NETCONF server and client for network management.

Features:
- NETCONF server with Unix domain socket transport
- NETCONF client with TUI interface
- LLDP support for network discovery
- YANG schema support
- FreeBSD-specific network interface management

Installation:
  pkg install ${PACKAGE_NAME}

Usage:
  netd --help    # Server help
  netc --help    # Client help

Documentation:
  ${PACKAGE_WEBSITE}
")

    file(WRITE "${PACKAGE_META}/+DISPLAY" "${DISPLAY_CONTENT}")

    # Create +MTREE_DIRS file
    set(MTREE_CONTENT "#mtree
/set type=file uid=0 gid=0 mode=644
${CMAKE_INSTALL_PREFIX}/sbin/netd type=file mode=755
${CMAKE_INSTALL_PREFIX}/bin/netc type=file mode=755
${CMAKE_INSTALL_PREFIX}/lib/netd type=dir mode=755
${CMAKE_INSTALL_PREFIX}/lib/pkgconfig type=dir mode=755
${CMAKE_INSTALL_PREFIX}/include/netd type=dir mode=755
${CMAKE_INSTALL_PREFIX}/share/netd type=dir mode=755
${CMAKE_INSTALL_PREFIX}/share/netd/yang type=dir mode=755
${CMAKE_INSTALL_PREFIX}/share/cmake/netd type=dir mode=755
")

    file(WRITE "${PACKAGE_META}/+MTREE_DIRS" "${MTREE_CONTENT}")

    # Create pre-install script
    set(PRE_INSTALL_CONTENT "#!/bin/sh
# Pre-install script for ${PACKAGE_NAME}

echo \"Installing ${PACKAGE_NAME} ${PACKAGE_VERSION}...\"

# Check if user netd exists, create if not
if ! id netd >/dev/null 2>&1; then
    echo \"Creating netd user...\"
    pw useradd -n netd -c \"NETD daemon\" -d /var/empty -s /usr/sbin/nologin
fi

# Create runtime directory
mkdir -p /var/run/netd
chown netd:netd /var/run/netd
chmod 755 /var/run/netd

exit 0
")

    file(WRITE "${PACKAGE_META}/+PRE_INSTALL" "${PRE_INSTALL_CONTENT}")

    # Create post-install script
    set(POST_INSTALL_CONTENT "#!/bin/sh
# Post-install script for ${PACKAGE_NAME}

echo \"${PACKAGE_NAME} ${PACKAGE_VERSION} installed successfully\"
echo \"\"
echo \"To start the NETD server:\"
echo \"  service netd start\"
echo \"\"
echo \"To connect with the client:\"
echo \"  netc\"
echo \"\"
echo \"For more information, see: ${PACKAGE_WEBSITE}\"

exit 0
")

    file(WRITE "${PACKAGE_META}/+POST_INSTALL" "${POST_INSTALL_CONTENT}")

    # Create pre-deinstall script
    set(PRE_DEINSTALL_CONTENT "#!/bin/sh
# Pre-deinstall script for ${PACKAGE_NAME}

echo \"Removing ${PACKAGE_NAME}...\"

# Stop service if running
if service netd status >/dev/null 2>&1; then
    echo \"Stopping netd service...\"
    service netd stop
fi

exit 0
")

    file(WRITE "${PACKAGE_META}/+PRE_DEINSTALL" "${PRE_DEINSTALL_CONTENT}")

    # Create post-deinstall script
    set(POST_DEINSTALL_CONTENT "#!/bin/sh
# Post-deinstall script for ${PACKAGE_NAME}

echo \"${PACKAGE_NAME} removed successfully\"

# Remove runtime directory if empty
if [ -d /var/run/netd ] && [ -z \"\$(ls -A /var/run/netd)\" ]; then
    rmdir /var/run/netd
fi

exit 0
")

    file(WRITE "${PACKAGE_META}/+POST_DEINSTALL" "${POST_DEINSTALL_CONTENT}")

    # Make scripts executable
    file(CHMOD ${PACKAGE_META}/+PRE_INSTALL PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
    file(CHMOD ${PACKAGE_META}/+POST_INSTALL PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
    file(CHMOD ${PACKAGE_META}/+PRE_DEINSTALL PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
    file(CHMOD ${PACKAGE_META}/+POST_DEINSTALL PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)

    # Create package metadata target
    add_custom_target(freebsd_package_metadata
        COMMAND ${CMAKE_COMMAND} -E echo "Creating FreeBSD package metadata..."
        COMMAND ${CMAKE_COMMAND} -E echo "Package directory: ${PACKAGE_DIR}"
        COMMAND ${CMAKE_COMMAND} -E echo "Package name: ${PACKAGE_NAME}-${PACKAGE_VERSION}"
        COMMAND ${CMAKE_COMMAND} -E echo "Package root: ${PACKAGE_ROOT}"
        COMMAND ${CMAKE_COMMAND} -E echo "Package meta: ${PACKAGE_META}"
        COMMAND ${CMAKE_COMMAND} -E echo "FreeBSD package metadata created successfully"
        COMMAND ${CMAKE_COMMAND} -E echo "To create the complete package, run:"
        COMMAND ${CMAKE_COMMAND} -E echo "  make freebsd_package"
        COMMENT "Creating FreeBSD package metadata"
    )

    # Create package target
    add_custom_target(freebsd_package
        COMMAND ${CMAKE_COMMAND} -E echo "Installing files to package root..."
        COMMAND ${CMAKE_COMMAND} -DCOMPONENT=netd -P cmake_install.cmake
        COMMAND ${CMAKE_COMMAND} -DCOMPONENT=netc -P cmake_install.cmake
        COMMAND ${CMAKE_COMMAND} -DCOMPONENT=shared -P cmake_install.cmake
        COMMAND ${CMAKE_COMMAND} -E echo "Creating FreeBSD package..."
        COMMAND ${PKG_CMD} create -m ${PACKAGE_META} -r ${PACKAGE_ROOT} -o ${CMAKE_BINARY_DIR}
        COMMAND ${CMAKE_COMMAND} -E echo "Package created: ${CMAKE_BINARY_DIR}/${PACKAGE_NAME}-${PACKAGE_VERSION}.pkg"
        DEPENDS freebsd_package_metadata
        COMMENT "Creating FreeBSD package"
    )

endfunction()

# Call the function to create the package targets
create_freebsd_package()
