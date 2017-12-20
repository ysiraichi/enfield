add_subdirectory (Allocators)

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
