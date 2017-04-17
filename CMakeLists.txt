cmake_minimum_required (VERSION 3.5.1)

project (enfield)

set (CMAKE_CXX_FLAGS "-O0 -std=c++11 -g")

include_directories (include)
add_subdirectory (lib)
add_subdirectory (tools)
