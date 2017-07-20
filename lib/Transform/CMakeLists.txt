add_library (EfdTransform
    QModule.cpp
    IdTable.cpp
    DependencyBuilderPass.cpp
    FlattenPass.cpp
    Utils.cpp
    InlineAllPass.cpp
    DynProgQbitAllocator.cpp
    QbitAllocator.cpp
    RenameQbitsPass.cpp
    ReverseEdgesPass.cpp
    WeightedPMQbitAllocator.cpp)
