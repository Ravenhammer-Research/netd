# GetBuildType.cmake - Build type validation and setup

# Set default build type to DEBUG if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "DEBUG" CACHE STRING "Build type" FORCE)
endif()

# Validate build type
if(NOT CMAKE_BUILD_TYPE MATCHES "^(DEBUG|RELEASE)$")
    message(FATAL_ERROR "Invalid build type: ${CMAKE_BUILD_TYPE}. Must be DEBUG or RELEASE")
endif()

# For RELEASE builds, ensure working directory is clean
if(CMAKE_BUILD_TYPE STREQUAL "RELEASE")
    if(GIT_REVISION STREQUAL "unknown")
        message(FATAL_ERROR "RELEASE builds require git to be available")
    endif()
    
    # Check if working directory is dirty
    execute_process(
        COMMAND git diff-index --quiet HEAD --
        RESULT_VARIABLE GIT_DIRTY_RESULT
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_QUIET
        ERROR_QUIET
    )
    
    if(NOT GIT_DIRTY_RESULT EQUAL 0)
        message(FATAL_ERROR "RELEASE builds require a clean working directory (no uncommitted changes)")
    endif()
    
    message(STATUS "RELEASE build: Working directory is clean")
endif()
