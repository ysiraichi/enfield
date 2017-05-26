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

efd_test (OneRestrictionSwapFinderTests
    EfdSupport)

# ==-------- Analysis ----------==
efd_test (ASTNodeTests
    EfdAnalysis EfdSupport)

efd_test (DriverTests
    EfdAnalysis EfdSupport)

efd_test (DriverFileTests
    EfdAnalysis EfdSupport)

# ==-------- Arch ----------==
efd_test (ArchGraphTests
    EfdArch EfdAnalysis EfdCore EfdSupport)

efd_test (IBMQX2Tests
    EfdArch EfdAnalysis EfdCore EfdSupport)

# ==-------- Transforms ----------==
efd_test (QModuleTests
    EfdTransform EfdAnalysis EfdCore EfdSupport)

efd_test (IdTableTests
    EfdTransform EfdAnalysis EfdCore EfdSupport)

efd_test (QModuleCloneTests
    EfdTransform EfdAnalysis EfdCore EfdSupport)

efd_test (DependencyBuilderPassTests
    EfdTransform EfdAnalysis EfdCore EfdSupport)

efd_test (FlattenPassTests
    EfdTransform EfdAnalysis EfdCore EfdSupport)

efd_test (TransformUtilsTests
    EfdTransform EfdAnalysis EfdCore EfdSupport)

efd_test (RenameQbitsPassTests
    EfdTransform EfdAnalysis EfdCore EfdSupport)

efd_test (DynProgQbitAllocatorTests
    EfdTransform EfdArch EfdAnalysis EfdCore EfdSupport)

