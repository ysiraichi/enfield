add_library (EfdTransform
    QModule.cpp
    QModulefyPass.cpp
    IdTable.cpp
    DependencyBuilderPass.cpp
    FlattenPass.cpp
    Utils.cpp
    InlineAllPass.cpp
    QbitAllocator.cpp
    DynProgQbitAllocator.cpp
    RenameQbitsPass.cpp
    ReverseEdgesPass.cpp)
