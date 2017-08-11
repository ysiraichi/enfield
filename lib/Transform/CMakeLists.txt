add_library (EfdTransform
    Pass.cpp
    QModule.cpp
    QbitToNumberPass.cpp
    DependencyBuilderPass.cpp
    FlattenPass.cpp
    Utils.cpp
    InlineAllPass.cpp
    RenameQbitsPass.cpp
    ReverseEdgesPass.cpp)

add_library(EfdAllocator
    QbitAllocator.cpp
    DynProgQbitAllocator.cpp
    SimpleQbitAllocator.cpp
    WeightedPMMappingFinder.cpp
    RandomMappingFinder.cpp
    PathGuidedDepSolver.cpp
    QbitterDepSolver.cpp)
