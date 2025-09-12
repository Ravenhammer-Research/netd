# GetGitRevisionDescription.cmake
# Get the current git hash and check if there are uncommitted changes

function(get_git_revision_description _var)
    # Check if git is available
    find_package(Git QUIET)
    if(NOT Git_FOUND)
        set(${_var} "unknown" PARENT_SCOPE)
        return()
    endif()

    # Get the current git hash (short version)
    execute_process(
        COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        RESULT_VARIABLE GIT_RESULT
        OUTPUT_VARIABLE GIT_HASH
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )

    # Check if git command succeeded
    if(NOT GIT_RESULT EQUAL 0)
        set(${_var} "unknown" PARENT_SCOPE)
        return()
    endif()

    # Check if there are uncommitted changes
    execute_process(
        COMMAND ${GIT_EXECUTABLE} diff-index --quiet HEAD --
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        RESULT_VARIABLE GIT_DIRTY_RESULT
        OUTPUT_QUIET
        ERROR_QUIET
    )

    # Append -dirty if there are uncommitted changes
    if(NOT GIT_DIRTY_RESULT EQUAL 0)
        set(GIT_HASH "${GIT_HASH}-dirty")
    endif()

    set(${_var} ${GIT_HASH} PARENT_SCOPE)
endfunction()

# Function to get full git hash
function(get_git_revision_description_long _var)
    # Check if git is available
    find_package(Git QUIET)
    if(NOT Git_FOUND)
        set(${_var} "unknown" PARENT_SCOPE)
        return()
    endif()

    # Get the current git hash (full version)
    execute_process(
        COMMAND ${GIT_EXECUTABLE} rev-parse HEAD
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        RESULT_VARIABLE GIT_RESULT
        OUTPUT_VARIABLE GIT_HASH
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )

    # Check if git command succeeded
    if(NOT GIT_RESULT EQUAL 0)
        set(${_var} "unknown" PARENT_SCOPE)
        return()
    endif()

    # Check if there are uncommitted changes
    execute_process(
        COMMAND ${GIT_EXECUTABLE} diff-index --quiet HEAD --
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        RESULT_VARIABLE GIT_DIRTY_RESULT
        OUTPUT_QUIET
        ERROR_QUIET
    )

    # Append -dirty if there are uncommitted changes
    if(NOT GIT_DIRTY_RESULT EQUAL 0)
        set(GIT_HASH "${GIT_HASH}-dirty")
    endif()

    set(${_var} ${GIT_HASH} PARENT_SCOPE)
endfunction()
