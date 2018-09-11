#ifndef __EFD_ALLOCATORS_H__
#define __EFD_ALLOCATORS_H__

#include "enfield/Arch/ArchGraph.h"
#include "enfield/Transform/Allocators/QbitAllocator.h"
#include "enfield/Support/Registry.h"
#include "enfield/Support/EnumString.h"

namespace efd {
    enum class Allocator {
        first,
#define EFD_ALLOCATOR(_Name_, _Class_)\
        Q_##_Name_,
#define EFD_ALLOCATOR_SIMPLE(_Name_, _Finder_, _Builder_)\
        Q_##_Name_,
#define EFD_ALLOCATOR_BMT(_Name_, _NCG_, _CS_, _PSS_, _SCE_, _LQPP_, _MSS_, _TSF_)\
        Q_##_Name_,
#include "enfield/Transform/Allocators/Allocators.def"
#undef EFD_ALLOCATOR
#undef EFD_ALLOCATOR_SIMPLE
#undef EFD_ALLOCATOR_BMT
        last
    };

    typedef EnumString<Allocator, Allocator::first, Allocator::last, 1> EnumAllocator;
    template <> std::vector<std::string> EnumAllocator::mStrVal;

    typedef Registry<QbitAllocator::uRef, ArchGraph::sRef, EnumAllocator> AllocatorRegistry;

    void InitializeAllQbitAllocators();
    /// \brief Returns true if there is an allocator mapped by \p key;
    bool HasAllocator(EnumAllocator key);
    /// \brief Registers an allocator, mapping \p name to \p ctor.
    void RegisterQbitAllocator(EnumAllocator key, AllocatorRegistry::CtorTy ctor);
    /// \brief Creates an allocator referenced by \p name with arguments \p arg.
    AllocatorRegistry::RetTy CreateQbitAllocator(EnumAllocator key, 
            AllocatorRegistry::ArgTy arg);


#define EFD_ALLOCATOR(_Name_, _Class_) \
    AllocatorRegistry::RetTy Create##_Class_(AllocatorRegistry::ArgTy arg);
#define EFD_ALLOCATOR_SIMPLE(_Name_, _Finder_, _Builder_) \
    AllocatorRegistry::RetTy Create##_Finder_##With##_Builder_\
    (AllocatorRegistry::ArgTy arg);
#define EFD_ALLOCATOR_BMT(_Name_, _NCG_, _CS_, _PSS_, _SCE_, _LQPP_, _MSS_, _TSF_) \
    AllocatorRegistry::RetTy CreateBMT##_NCG_##_CS_##_PSS_##_SCE_##_LQPP_##_MSS_##_TSF_\
    (AllocatorRegistry::ArgTy arg);
#include "enfield/Transform/Allocators/Allocators.def"
#undef EFD_ALLOCATOR
#undef EFD_ALLOCATOR_SIMPLE
#undef EFD_ALLOCATOR_BMT
}

#endif
