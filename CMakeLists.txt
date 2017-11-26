cmake_minimum_required (VERSION 3.5.1)

project (enfield)

set (DEFAULT_BUILD_TYPE "Debug")
set (CMAKE_CXX_FLAGS "-std=c++11")
set (CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

if (NOT CMAKE_BUILD_TYPE)
    set (CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type." FORCE)
endif()

if ("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
    set (COV_FLAGS "-fprofile-arcs -ftest-coverage")
    set (CMAKE_CXX_FLAGS "-O0 -g ${CMAKE_CXX_FLAGS} ${COV_FLAGS}")
elseif ("${CMAKE_BUILD_TYPE}" STREQUAL "Release")
    set (CMAKE_CXX_FLAGS "-O3 ${CMAKE_CXX_FLAGS}")
endif()

include_directories (include)

add_subdirectory (lib)
add_subdirectory (tools)

option (ENABLE_TESTS "Enables the tests." off)

if (ENABLE_TESTS)
    enable_testing ()
    include (Tests)
    add_subdirectory (tests)
endif()
