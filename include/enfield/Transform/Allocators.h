#ifndef __EFD_ALLOCATORS_H__
#define __EFD_ALLOCATORS_H__

#include "enfield/Arch/ArchGraph.h"
#include "enfield/Transform/QbitAllocator.h"

namespace efd {
    namespace alloc {
        typedef QbitAllocator::uRef RetTy;
        typedef ArchGraph::sRef ArgTy;
        typedef std::function<RetTy(ArgTy)> AllocCtorTy;
    }

    void InitializeAllQbitAllocators();
    /// \brief Returns true if there is an allocator referenced by \p name.
    void RegisterQbitAllocator(std::string name, alloc::AllocCtorTy ctor);
    /// \brief Creates an allocator referenced by \p name with arguments \p arg.
    alloc::RetTy CreateQbitAllocator(std::string name, alloc::ArgTy arg);


#define EFD_ALLOCATOR(_Name_, _Class_) \
    alloc::RetTy Create##_Class_(alloc::ArgTy arg);
#define EFD_ALLOCATOR_SIMPLE(_Name_, _Finder_, _Solver_) \
    alloc::RetTy Create##_Finder_##With##_Solver_(alloc::ArgTy arg);
#include "enfield/Transform/Allocators.def"
#undef EFD_ALLOCATOR
#undef EFD_ALLOCATOR_SIMPLE
}

#endif
