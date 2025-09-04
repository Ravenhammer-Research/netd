# Yacc CMake module for processing .y files

function(add_yacc_source target_name yacc_file)
    # Get the base name without extension
    get_filename_component(base_name ${yacc_file} NAME_WE)
    
    # Set output file names
    set(c_file "${CMAKE_CURRENT_BINARY_DIR}/${base_name}.c")
    set(h_file "${CMAKE_CURRENT_BINARY_DIR}/${base_name}.h")
    
    # Create custom command to generate .c and .h files from .y file
    add_custom_command(
        OUTPUT ${c_file} ${h_file}
        COMMAND yacc -d -o ${c_file} ${CMAKE_CURRENT_SOURCE_DIR}/${yacc_file}
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${yacc_file}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating parser from ${yacc_file}"
        VERBATIM
    )
    
    # Add generated files to target sources
    target_sources(${target_name} PRIVATE ${c_file})
    
    # Set include directories to find generated headers
    target_include_directories(${target_name} PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
endfunction()
