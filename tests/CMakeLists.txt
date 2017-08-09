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

efd_test (WeightedGraphTests
    EfdSupport)

efd_test (PartialMatchingFinderTests
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
    EfdArch EfdAnalysis EfdSupport)

efd_test (IBMQX2Tests
    EfdArch EfdAnalysis EfdSupport)

# ==-------- Transforms ----------==
efd_test (QModuleTests
    EfdTransform EfdAnalysis EfdSupport)

efd_test (QModuleCloneTests
    EfdTransform EfdAnalysis EfdSupport)

efd_test (DependencyBuilderPassTests
    EfdTransform EfdAnalysis EfdSupport)

efd_test (FlattenPassTests
    EfdTransform EfdAnalysis EfdSupport)

efd_test (TransformUtilsTests
    EfdTransform EfdAnalysis EfdSupport)

efd_test (InlineAllPassTests
    EfdTransform EfdAnalysis EfdSupport)

efd_test (RenameQbitsPassTests
    EfdTransform EfdAnalysis EfdSupport)

efd_test (ReverseEdgesPassTests
    EfdAllocator EfdTransform EfdArch EfdAnalysis EfdSupport)

# ==-------- Allocator ----------==
efd_test (DynProgQbitAllocatorTests
    EfdAllocator EfdTransform EfdArch EfdAnalysis EfdSupport)

efd_test (WeightedPMQbitAllocatorTests
    EfdAllocator EfdTransform EfdArch EfdAnalysis EfdSupport)

efd_test (QbitterQbitAllocatorTests
    EfdAllocator EfdTransform EfdArch EfdAnalysis EfdSupport)

