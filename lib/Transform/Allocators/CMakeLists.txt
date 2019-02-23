add_subdirectory (BMT)
add_subdirectory (Simple)

add_library (EfdAllocator
    Allocators.cpp
    QbitAllocator.cpp
    StdSolutionQAllocator.cpp
    DynprogQAllocator.cpp
    SimpleQAllocator.cpp
    GreedyCktQAllocator.cpp
    IBMQAllocator.cpp
    BoundedMappingTreeQAllocator.cpp
    SabreQAllocator.cpp
    JKUQAllocator.cpp
    OptBMTQAllocator.cpp
    LayeredBMTQAllocator.cpp
    ChallengeWinnerQAllocator.cpp)
