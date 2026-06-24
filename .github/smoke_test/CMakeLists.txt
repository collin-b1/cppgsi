cmake_minimum_required(VERSION 3.14)
project(cppgsi_smoke_test LANGUAGES CXX)

find_package(cppgsi REQUIRED)

add_executable(smoke_test main.cpp)
target_link_libraries(smoke_test PRIVATE cppgsi::cppgsi)
