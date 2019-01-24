add_executable (efd Enfield.cpp)
target_link_libraries (efd
    EfdArch EfdAllocator EfdBMTImpl EfdSimpleImpl
    EfdTransform EfdAnalysis EfdSupport
    ${JSONCPP_MAIN})

add_executable (inliner Inliner.cpp)
target_link_libraries (inliner
    EfdTransform EfdAllocator EfdTransform EfdAllocator EfdArch EfdBMTImpl EfdSimpleImpl
    EfdAnalysis EfdSupport
    ${JSONCPP_MAIN})

add_executable (gen-prog Generator.cpp)
target_link_libraries (gen-prog
    EfdArch EfdAllocator EfdBMTImpl EfdSimpleImpl
    EfdTransform EfdAnalysis EfdSupport
    ${JSONCPP_MAIN})
