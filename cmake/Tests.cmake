find_package (GTest REQUIRED)
include_directories (${GTEST_INCLUDE_DIRS})

set (GTEST_LIBS ${GTEST_BOTH_LIBRARIES})
set (UTIL_LIBS "-lpthread")

function (efd_test TestName)
    add_executable (${TestName} 
        "${TestName}.cpp")

    target_link_libraries (${TestName} 
        ${ARGN} 
        ${GTEST_LIBS}
        ${UTIL_LIBS}
        ${JSONCPP_MAIN})

    add_test (${TestName} ${TestName})
endfunction ()
