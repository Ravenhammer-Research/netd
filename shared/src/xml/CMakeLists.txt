# XML parsing and generation components

add_library(netd_shared_xml
  base.cpp
  envelope.cpp
  hello.cpp
)

target_include_directories(netd_shared_xml PRIVATE ${GLOBAL_INCLUDE_DIRS})

target_link_libraries(netd_shared_xml
  netd_shared
)

target_include_directories(netd_shared_xml PUBLIC
  ${CMAKE_SOURCE_DIR}/shared/include
)
