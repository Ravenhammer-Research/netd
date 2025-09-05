# Marshalling module sources
set(MARSHALLING_SOURCES
  error.cpp
  data.cpp
  network.cpp
  interface.cpp
  route.cpp
  filter.cpp
)

# Create the marshalling library
add_library(netd_marshalling STATIC ${MARSHALLING_SOURCES})

# Link with libyang
target_link_libraries(netd_marshalling yang)

# Set include directories
target_include_directories(netd_marshalling
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}>
    $<BUILD_INTERFACE:${LIBYANG_INCLUDE_DIRS}>
    $<INSTALL_INTERFACE:include>
)

# Set compile definitions
target_compile_definitions(netd_marshalling
  PRIVATE
    -DNETD_MARSHALLING_EXPORTS
)
