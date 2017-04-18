cmake_minimum_required (VERSION 3.5.1)

project (enfield)

set (CMAKE_CXX_FLAGS "-O0 -std=c++11 -g")
set (CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

include_directories (include)

add_subdirectory (lib)
add_subdirectory (tools)

option (ENABLE_TESTS "Enables the tests." off)

if (ENABLE_TESTS)
    enable_testing ()
    include (Tests)
    add_subdirectory (tests)
endif()
