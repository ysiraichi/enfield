add_library (EfdAllocator
    Allocators.cpp
    QbitAllocator.cpp
    StdSolutionQAllocator.cpp
    DepSolverQAllocator.cpp
    DynprogDepSolver.cpp
    SimpleDepSolver.cpp
    BoundedSIDepSolver.cpp
    WeightedSIMappingFinder.cpp
    IdentityMappingFinder.cpp
    RandomMappingFinder.cpp
    PathGuidedSolBuilder.cpp
    QbitterSolBuilder.cpp
    GreedyCktQAllocator.cpp
    IBMQAllocator.cpp
    BoundedMappingTreeQAllocator.cpp
    DefaultBMTQAllocatorImpl.cpp)
