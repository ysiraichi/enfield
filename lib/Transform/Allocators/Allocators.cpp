#include "enfield/Transform/Allocators/Allocators.h"
#include "enfield/Transform/Allocators/DynprogQAllocator.h"
#include "enfield/Transform/Allocators/GreedyCktQAllocator.h"
#include "enfield/Transform/Allocators/IBMQAllocator.h"
#include "enfield/Transform/Allocators/SimpleQAllocator.h"
#include "enfield/Transform/Allocators/BoundedMappingTreeQAllocator.h"

#include "enfield/Transform/Allocators/Simple/WeightedSIMappingFinder.h"
#include "enfield/Transform/Allocators/Simple/RandomMappingFinder.h"
#include "enfield/Transform/Allocators/Simple/IdentityMappingFinder.h"
#include "enfield/Transform/Allocators/Simple/PathGuidedSolBuilder.h"
#include "enfield/Transform/Allocators/Simple/QbitterSolBuilder.h"

#include <unordered_map>
#include <functional>

namespace efd {
    typedef std::shared_ptr<efd::AllocatorRegistry> AllocatorRegistryPtr;
}

template <> std::vector<std::string> efd::EnumAllocator::mStrVal {
#define EFD_FIRSTLAST(_First_, _Last_)
#define EFD_ALLOCATOR(_Name_, _Class_) \
    "Q_"#_Name_,
#define EFD_ALLOCATOR_SIMPLE(_Name_, _Finder_, _Builder_) \
    "Q_"#_Name_,
#include "enfield/Transform/Allocators/Allocators.def"
#undef EFD_FIRSTLAST
#undef EFD_ALLOCATOR
#undef EFD_ALLOCATOR_SIMPLE
};

// -------------- Static -----------------

static efd::AllocatorRegistryPtr Registry(nullptr);
static efd::AllocatorRegistryPtr GetRegistry() {
    if (Registry.get() == nullptr)
        Registry.reset(new efd::AllocatorRegistry());
    return Registry;
}

// -------------- Public Functions -----------------
void efd::InitializeAllQbitAllocators() {
#define EFD_FIRSTLAST(_First_, _Last_)
#define EFD_ALLOCATOR(_Name_, _Class_) \
    RegisterQbitAllocator(Allocator::Q_##_Name_, Create##_Class_);
#define EFD_ALLOCATOR_SIMPLE(_Name_, _Finder_, _Builder_) \
    RegisterQbitAllocator(Allocator::Q_##_Name_, Create##_Finder_##With##_Builder_);
#include "enfield/Transform/Allocators/Allocators.def"
#undef EFD_FIRSTLAST
#undef EFD_ALLOCATOR
#undef EFD_ALLOCATOR_SIMPLE
}

bool efd::HasAllocator(EnumAllocator key) {
    return GetRegistry()->hasObj(key);
}

void efd::RegisterQbitAllocator(EnumAllocator key, AllocatorRegistry::CtorTy ctor) {
    GetRegistry()->registerObj(key, ctor);
}

efd::AllocatorRegistry::RetTy
efd::CreateQbitAllocator(EnumAllocator key, AllocatorRegistry::ArgTy arg) {
    return GetRegistry()->createObj(key, arg);
}

// -------------- Allocator Functions -----------------
#define EFD_FIRSTLAST(_First_, _Last_)
#define EFD_ALLOCATOR(_Name_, _Class_) \
    efd::AllocatorRegistry::RetTy efd::Create##_Class_(AllocatorRegistry::ArgTy arg) {\
        return _Class_::Create(arg);\
    }
#define EFD_ALLOCATOR_SIMPLE(_Name_, _Finder_, _Builder_) \
    efd::AllocatorRegistry::RetTy\
    efd::Create##_Finder_##With##_Builder_(AllocatorRegistry::ArgTy arg) {\
        auto allocator = SimpleQAllocator::Create(arg);\
        allocator->setMapFinder(_Finder_::Create());\
        allocator->setSolBuilder(_Builder_::Create());\
        return std::move(allocator);\
    }
#include "enfield/Transform/Allocators/Allocators.def"
#undef EFD_FIRSTLAST
#undef EFD_ALLOCATOR
#undef EFD_ALLOCATOR_SIMPLE

