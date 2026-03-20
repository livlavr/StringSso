cmake_minimum_required(VERSION 3.20)
project(StringSso)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(string-sso INTERFACE)

target_include_directories(string-sso INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

enable_testing()
add_subdirectory(tests)
