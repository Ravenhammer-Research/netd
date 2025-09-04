# Client source CMakeLists.txt

# Add source files to the client executable
target_sources(netd_client
    PRIVATE
        main.cpp
        netconf.cpp
        terminal.cpp
)
