# CMakeLists.txt for xml module
# Copyright (c) 2025 Paige Thompson / Raven Hammer Research

set(XML_SOURCES
    utils.c
    vrf.c
    edit.c
)

# Create xml library
add_library(xml STATIC ${XML_SOURCES})

# Set include directories
target_include_directories(xml PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_SOURCE_DIR}/../..
)

# Link dependencies
target_link_libraries(xml
    netd
)



# Install library
install(TARGETS xml
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
) 