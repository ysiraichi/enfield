file (GLOB files "files/*")

foreach (qfile ${files})
    get_filename_component (qfilename ${qfile} NAME)
    configure_file ("files/${qfilename}" 
        "${CMAKE_CURRENT_BINARY_DIR}/files/${qfilename}" COPYONLY)
endforeach (qfile)

efd_test (CommandLineTests
    EfdSupport)

efd_test (ASTNodeTests
    EfdAnalysis EfdSupport)

efd_test (DriverTests
    EfdAnalysis EfdSupport)

efd_test (DriverFileTests
    EfdAnalysis EfdSupport)

efd_test (QModuleTests
    EfdAnalysis EfdSupport)

efd_test (IdTableTests
    EfdAnalysis EfdSupport)

efd_test (DependencyBuilderPassTests
    EfdAnalysis EfdSupport)

efd_test (QModuleCloneTests
    EfdAnalysis EfdSupport)
