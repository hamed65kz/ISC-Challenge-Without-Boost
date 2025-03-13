function(configure_vcpkg)
    if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux" OR ${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
        message("Platform is Linux/MacOs, calling bash script")
        execute_process(
            WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/cmake_modules/configure_vcpkg/scripts/"
            COMMAND  bash "configure_vcpkg.sh" "${CMAKE_SOURCE_DIR}/Dependencies.txt"
            RESULT_VARIABLE script_result
            #COMMAND_ECHO STDOUT
        )
    elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
        message("Platform is Windows, calling batch file")
        #if both script path and script argument have whitespace, cmake can thandle them with qouting.
        #I use WORKING_DIRECTORY to bypass this problem
        execute_process(
            WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/cmake_modules/configure_vcpkg/scripts/"
            COMMAND  "configure_vcpkg.bat" "../../../Dependencies.txt"
            RESULT_VARIABLE script_result
            #COMMAND_ECHO STDOUT
        )
    else()
        message(FATAL_ERROR "Unsupported platform: ${CMAKE_SYSTEM_NAME}")
    endif()

    if(NOT script_result EQUAL 0)
        message(FATAL_ERROR "Script failed with error code: ${script_result}")
    endif()
endfunction()