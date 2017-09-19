#ifndef __EFD_ALLOCATORS_H__
#define __EFD_ALLOCATORS_H__

#include "enfield/Arch/ArchGraph.h"
#include "enfield/Transform/QbitAllocator.h"
#include "enfield/Support/Registry.h"

namespace efd {
    typedef Registry<QbitAllocator::uRef, ArchGraph::sRef> AllocatorRegistry;

    void InitializeAllQbitAllocators();
    /// \brief Returns true if there is an allocator mapped by \p key;
    bool HasAllocator(std::string key);
    /// \brief Registers an allocator, mapping \p name to \p ctor.
    void RegisterQbitAllocator(std::string key, AllocatorRegistry::CtorTy ctor);
    /// \brief Creates an allocator referenced by \p name with arguments \p arg.
    AllocatorRegistry::RetTy CreateQbitAllocator(std::string key, 
            AllocatorRegistry::ArgTy arg);


#define EFD_ALLOCATOR(_Name_, _Class_) \
    AllocatorRegistry::RetTy Create##_Class_(AllocatorRegistry::ArgTy arg);
#define EFD_ALLOCATOR_SIMPLE(_Name_, _Finder_, _Builder_) \
    AllocatorRegistry::RetTy Create##_Finder_##With##_Builder_\
    (AllocatorRegistry::ArgTy arg);
#include "enfield/Transform/Allocators.def"
#undef EFD_ALLOCATOR
#undef EFD_ALLOCATOR_SIMPLE
}

#endif
