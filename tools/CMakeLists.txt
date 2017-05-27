add_executable (efd Enfield.cpp)
target_link_libraries (efd
    EfdArch EfdTransform EfdAnalysis EfdCore EfdSupport)
