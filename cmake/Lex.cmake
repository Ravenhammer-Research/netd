# Lex CMake module for processing .l files

function(add_lex_source target_name lex_file)
    # Get the base name without extension
    get_filename_component(base_name ${lex_file} NAME_WE)
    
    # Set output file name
    set(c_file "${CMAKE_CURRENT_BINARY_DIR}/${base_name}.c")
    
    # Create custom command to generate .c file from .l file
    add_custom_command(
        OUTPUT ${c_file}
        COMMAND flex -o ${c_file} ${CMAKE_CURRENT_SOURCE_DIR}/${lex_file}
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${lex_file}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating lexer from ${lex_file}"
        VERBATIM
    )
    
    # Add generated file to target sources
    target_sources(${target_name} PRIVATE ${c_file})
endfunction()
