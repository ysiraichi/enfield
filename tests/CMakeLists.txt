file (GLOB files "files/*.qasm")
message (STATUS "${files}")

foreach (f ${files})
    message (STATUS "${f}")
endforeach (f)

efd_test (CommandLineTests
    EfdSupport)

efd_test (ASTNodeTests
    EfdAnalysis EfdSupport)

efd_test (DriverTests
    EfdAnalysis EfdSupport)

efd_test (DriverFileTests
    EfdAnalysis EfdSupport)
