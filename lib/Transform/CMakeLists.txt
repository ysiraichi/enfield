add_library (EfdTransform
    Pass.cpp
    QModule.cpp
    IdTable.cpp
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
    WeightedPMQbitAllocator.cpp
    RandomQbitAllocator.cpp)
