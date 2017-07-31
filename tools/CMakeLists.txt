add_executable (efd Enfield.cpp)
target_link_libraries (efd
    EfdArch EfdAllocator EfdTransform EfdAnalysis EfdSupport)

add_executable (gen-prog Generator.cpp)
target_link_libraries (gen-prog
    EfdArch EfdAllocator EfdTransform EfdAnalysis EfdSupport)
