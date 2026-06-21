cmake_minimum_required(VERSION 3.14)
project(cppgsi VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

# Third-party submodules
set(HTTPLIB_INSTALL OFF CACHE INTERNAL "")
add_subdirectory(third_party/httplib)
# httplib marks its includes as SYSTEM which suppresses CLion's indexer; clear it
set_target_properties(httplib PROPERTIES INTERFACE_SYSTEM_INCLUDE_DIRECTORIES "")

set(JSON_BuildTests OFF CACHE INTERNAL "")
set(JSON_Install OFF CACHE INTERNAL "")
add_subdirectory(third_party/nlohmann)

# Library target
add_library(cppgsi STATIC
        gsi/gsi_config.cpp
        gsi/models/models.cpp
        gsi/game_state.cpp
        gsi/gsi_dispatcher.cpp
        gsi/gsi_server.cpp
)
target_include_directories(cppgsi PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/third_party/httplib>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/third_party/nlohmann/include>
        $<INSTALL_INTERFACE:include>
)
target_link_libraries(cppgsi PUBLIC httplib::httplib nlohmann_json::nlohmann_json)
target_compile_features(cppgsi PUBLIC cxx_std_23)

# GSI server executable
add_executable(cppgsi_server test.cpp)
target_link_libraries(cppgsi_server PRIVATE cppgsi)
