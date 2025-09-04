# Server source CMakeLists.txt

# Add source files to the server executable
target_sources(netd_server
    PRIVATE
        main.cpp
        netconf.cpp
        store.cpp
)
