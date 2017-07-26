add_library (EfdTransform
    Pass.cpp
    QModule.cpp
    IdTable.cpp
    QbitToNumberPass.cpp
    DependencyBuilderPass.cpp
    FlattenPass.cpp
    Utils.cpp
    InlineAllPass.cpp
    DynProgQbitAllocator.cpp
    QbitAllocator.cpp
    RenameQbitsPass.cpp
    ReverseEdgesPass.cpp
    WeightedPMQbitAllocator.cpp)
