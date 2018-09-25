add_subdirectory (Allocators)

add_library (EfdTransform
    ArchVerifierPass.cpp
    CircuitGraph.cpp
    CircuitGraphBuilderPass.cpp
    CNOTLBOWrapperPass.cpp
    DependencyBuilderPass.cpp
    DependencyGraphBuilderPass.cpp
    Driver.cpp
    ErrorRateCalculationPass.cpp
    FlattenPass.cpp
    InlineAllPass.cpp
    LayersBuilderPass.cpp
    LayerBasedOrderingWrapperPass.cpp
    Pass.cpp
    PassCache.cpp
    QModule.cpp
    QModuleQualityEvalPass.cpp
    QubitRemapPass.cpp
    RenameQbitsPass.cpp
    ReverseEdgesPass.cpp
    SemanticVerifierPass.cpp
    Utils.cpp
    XbitToNumberPass.cpp)
