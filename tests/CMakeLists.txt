file (GLOB files "files/*")

foreach (qfile ${files})
    get_filename_component (qfilename ${qfile} NAME)
    configure_file ("files/${qfilename}" 
        "${CMAKE_CURRENT_BINARY_DIR}/files/${qfilename}" COPYONLY)
endforeach (qfile)

# ==-------- Support ----------==
efd_test (CommandLineTests
    EfdSupport)

efd_test (GraphTests
    EfdSupport)

# ==-------- Analysis ----------==
efd_test (ASTNodeTests
    EfdAnalysis EfdSupport)

efd_test (DriverTests
    EfdAnalysis EfdSupport)

efd_test (DriverFileTests
    EfdAnalysis EfdSupport)

efd_test (QModuleTests
    EfdAnalysis EfdCore EfdSupport)

efd_test (IdTableTests
    EfdAnalysis EfdCore EfdSupport)

efd_test (QModuleCloneTests
    EfdAnalysis EfdCore EfdSupport)

# ==-------- Transforms ----------==
efd_test (DependencyBuilderPassTests
    EfdTransform EfdAnalysis EfdCore EfdSupport)

efd_test (FlattenPassTests
    EfdTransform EfdAnalysis EfdCore EfdSupport)

efd_test (GateInlineTests
    EfdTransform EfdAnalysis EfdCore EfdSupport)

