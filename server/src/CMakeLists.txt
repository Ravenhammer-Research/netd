# Server source CMakeLists.txt

# Add subdirectories
add_subdirectory(netconf)
add_subdirectory(store)

# Add source files to the server executable
target_sources(netd_server
    PRIVATE
        main.cpp
)
