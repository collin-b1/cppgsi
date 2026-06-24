cmake_minimum_required(VERSION 3.14)
project(cppgsi VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Detect whether we're the root project or being consumed as a dependency.
if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    set(CPPGSI_IS_TOP_LEVEL ON)
else()
    set(CPPGSI_IS_TOP_LEVEL OFF)
endif()

if(CPPGSI_IS_TOP_LEVEL)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)
endif()

# Third-party: cpp-httplib (header-only, no CMake target needed)
# Third-party: nlohmann/json (header-only, no CMake target needed)
# Include paths are set directly on cppgsi below; no add_subdirectory required.

# Library target
add_library(cppgsi STATIC
        gsi/gsi_config.cpp
        gsi/models/models.cpp
        gsi/game_state.cpp
        gsi/gsi_dispatcher.cpp
        gsi/gsi_server.cpp
)
# Namespaced alias — use cppgsi::cppgsi in all consumers (FetchContent, submodule, installed).
add_library(cppgsi::cppgsi ALIAS cppgsi)

target_include_directories(cppgsi PUBLIC
        # Build tree: root for "gsi/..." includes; third_party/ parent so that
        # #include "httplib/httplib.h" resolves to third_party/httplib/httplib.h.
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/third_party>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/third_party/nlohmann/include>
        # Install tree: all headers land under <prefix>/include/.
        $<INSTALL_INTERFACE:include>
)
target_compile_features(cppgsi PUBLIC cxx_std_23)

# Tests — only when building this project directly, not as a dependency.
if(CPPGSI_IS_TOP_LEVEL)
    enable_testing()
    add_subdirectory(tests)
endif()

# Examples — not built by default so they don't affect library consumers.
# Enable with: cmake -DCPPGSI_BUILD_EXAMPLES=ON
option(CPPGSI_BUILD_EXAMPLES "Build cppgsi examples" OFF)
if(CPPGSI_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# ---- Install support ---------------------------------------------------------

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

install(TARGETS cppgsi
        EXPORT cppgsiTargets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

# Library headers: gsi/ -> <prefix>/include/gsi/
install(DIRECTORY gsi/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/gsi
        FILES_MATCHING PATTERN "*.h"
)

# Bundled httplib: <prefix>/include/httplib/httplib.h
# Matches the #include "httplib/httplib.h" used in gsi_server.h.
install(FILES third_party/httplib/httplib.h
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/httplib
)

# Bundled nlohmann/json: <prefix>/include/nlohmann/
install(DIRECTORY third_party/nlohmann/include/nlohmann
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        FILES_MATCHING PATTERN "*.hpp"
)

install(EXPORT cppgsiTargets
        FILE cppgsiTargets.cmake
        NAMESPACE cppgsi::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/cppgsi
)

configure_package_config_file(
        cmake/cppgsiConfig.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/cppgsiConfig.cmake
        INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/cppgsi
)

write_basic_package_version_file(
        ${CMAKE_CURRENT_BINARY_DIR}/cppgsiConfigVersion.cmake
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY SameMajorVersion
)

install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/cppgsiConfig.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/cppgsiConfigVersion.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/cppgsi
)
