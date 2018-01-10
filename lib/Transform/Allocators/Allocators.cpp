#include "enfield/Transform/Allocators/Allocators.h"
#include "enfield/Transform/Allocators/DynprogDepSolver.h"
#include "enfield/Transform/Allocators/SimpleDepSolver.h"
#include "enfield/Transform/Allocators/WeightedSIMappingFinder.h"
#include "enfield/Transform/Allocators/RandomMappingFinder.h"
#include "enfield/Transform/Allocators/IdentityMappingFinder.h"
#include "enfield/Transform/Allocators/PathGuidedSolBuilder.h"
#include "enfield/Transform/Allocators/QbitterSolBuilder.h"

#include <unordered_map>
#include <functional>

namespace efd {
    typedef std::shared_ptr<efd::AllocatorRegistry> AllocatorRegistryPtr;
}

// -------------- Static -----------------

static efd::AllocatorRegistryPtr Registry(nullptr);
static efd::AllocatorRegistryPtr GetRegistry() {
    if (Registry.get() == nullptr)
        Registry.reset(new efd::AllocatorRegistry());
    return Registry;
}

// -------------- Public Functions -----------------
void efd::InitializeAllQbitAllocators() {
#define EFD_ALLOCATOR(_Name_, _Class_) \
    RegisterQbitAllocator(#_Name_, Create##_Class_);
#define EFD_ALLOCATOR_SIMPLE(_Name_, _Finder_, _Builder_) \
    RegisterQbitAllocator(#_Name_, Create##_Finder_##With##_Builder_);
#include "enfield/Transform/Allocators/Allocators.def"
#undef EFD_ALLOCATOR
#undef EFD_ALLOCATOR_SIMPLE
}

bool efd::HasAllocator(std::string key) {
    return GetRegistry()->hasObj(key);
}

void efd::RegisterQbitAllocator(std::string key, AllocatorRegistry::CtorTy ctor) {
    GetRegistry()->registerObj(key, ctor);
}

efd::AllocatorRegistry::RetTy
efd::CreateQbitAllocator(std::string key, AllocatorRegistry::ArgTy arg) {
    return GetRegistry()->createObj(key, arg);
}

// -------------- Allocator Functions -----------------
#define EFD_ALLOCATOR(_Name_, _Class_) \
    efd::AllocatorRegistry::RetTy efd::Create##_Class_(AllocatorRegistry::ArgTy arg) {\
        return _Class_::Create(arg);\
    }
#define EFD_ALLOCATOR_SIMPLE(_Name_, _Finder_, _Builder_) \
    efd::AllocatorRegistry::RetTy\
    efd::Create##_Finder_##With##_Builder_(AllocatorRegistry::ArgTy arg) {\
        auto allocator = SimpleDepSolver::Create(arg);\
        allocator->setMapFinder(_Finder_::Create());\
        allocator->setSolBuilder(_Builder_::Create());\
        return std::move(allocator);\
    }
#include "enfield/Transform/Allocators/Allocators.def"
#undef EFD_ALLOCATOR
#undef EFD_ALLOCATOR_SIMPLE

