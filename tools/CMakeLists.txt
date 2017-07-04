add_executable (efd Enfield.cpp)
target_link_libraries (efd
    EfdArch EfdTransform EfdAnalysis EfdCore EfdSupport)

add_executable (gen-prog Generator.cpp)
target_link_libraries (gen-prog
    EfdArch EfdTransform EfdAnalysis EfdCore EfdSupport)
