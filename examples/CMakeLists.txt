cmake_minimum_required(VERSION 3.14)
project(cppgsi_examples LANGUAGES CXX)

# When built as part of the cppgsi repo (add_subdirectory), the cppgsi target
# already exists. If you ever extract this folder as a standalone project,
# replace the line below with find_package(cppgsi REQUIRED) once the library
# ships a CMake install config.
if(NOT TARGET cppgsi)
    add_subdirectory(.. cppgsi_root)
endif()

function(add_example name)
    add_executable(${name} ${name}.cpp)
    target_link_libraries(${name} PRIVATE cppgsi)
    set_target_properties(${name} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/examples
    )
endfunction()

add_example(quickstart)
add_example(grenades)
add_example(round_events)
