add_library (EfdTransform
    Pass.cpp
    PassCache.cpp
    QModule.cpp
    XbitToNumberPass.cpp
    DependencyBuilderPass.cpp
    FlattenPass.cpp
    Utils.cpp
    InlineAllPass.cpp
    RenameQbitsPass.cpp
    ReverseEdgesPass.cpp
    CircuitGraphBuilderPass.cpp
    LayerBasedOrderingWrapperPass.cpp
    CNOTLBOWrapperPass.cpp)

add_library(EfdAllocator
    Allocators.cpp
    QbitAllocator.cpp
    DynProgQbitAllocator.cpp
    SimpleQbitAllocator.cpp
    WeightedPMMappingFinder.cpp
    IdentityMappingFinder.cpp
    RandomMappingFinder.cpp
    PathGuidedSolBuilder.cpp
    QbitterSolBuilder.cpp)
