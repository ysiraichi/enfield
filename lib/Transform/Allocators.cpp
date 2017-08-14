#include "enfield/Transform/Allocators.h"
#include "enfield/Transform/DynProgQbitAllocator.h"
#include "enfield/Transform/SimpleQbitAllocator.h"
#include "enfield/Transform/WeightedPMMappingFinder.h"
#include "enfield/Transform/RandomMappingFinder.h"
#include "enfield/Transform/IdentityMappingFinder.h"
#include "enfield/Transform/PathGuidedDepSolver.h"
#include "enfield/Transform/QbitterDepSolver.h"

#include <unordered_map>
#include <functional>

namespace efd {
    /// \brief Registry with the available qubit allocators.
    class AllocatorRegistry {
        private:
            std::unordered_map<std::string, alloc::AllocCtorTy> mAllocCtorMap;

        public:
            /// \brief Registers an allocator referenced by \p name and constructed by
            /// \p ctor.
            void registerAllocator(std::string name, alloc::AllocCtorTy ctor); 
            /// \brief Returns true if there is an allocator referenced by \p name.
            bool hasAllocator(std::string name) const;
            /// \brief Creates an allocator referenced by \p name with arguments \p arg.
            alloc::RetTy createAllocator(std::string name, alloc::ArgTy arg) const;
    };
}

void efd::AllocatorRegistry::registerAllocator
(std::string name, alloc::AllocCtorTy ctor) {
    assert(!hasAllocator(name) &&
            "Trying to register a qubit allocator with a name already used.");
    mAllocCtorMap[name] = ctor;
}

bool efd::AllocatorRegistry::hasAllocator(std::string name) const {
    return mAllocCtorMap.find(name) != mAllocCtorMap.end();
}

efd::alloc::RetTy
efd::AllocatorRegistry::createAllocator(std::string name, alloc::ArgTy arg) const {
    assert(hasAllocator(name) && "Trying to create a qubit allocator not registered.");
    return mAllocCtorMap.at(name)(arg);
}

// -------------- Static -----------------
typedef std::shared_ptr<efd::AllocatorRegistry> AllocatorRegistryPtr;

static AllocatorRegistryPtr Registry(nullptr);
static AllocatorRegistryPtr GetRegistry() {
    if (Registry.get() == nullptr)
        Registry.reset(new efd::AllocatorRegistry());
    return Registry;
}

// -------------- Public Functions -----------------
void efd::InitializeAllQbitAllocators() {
#define EFD_ALLOCATOR(_Name_, _Class_) \
    RegisterQbitAllocator(#_Name_, Create##_Class_);
#define EFD_ALLOCATOR_SIMPLE(_Name_, _Finder_, _Solver_) \
    RegisterQbitAllocator(#_Name_, Create##_Finder_##With##_Solver_);
#include "enfield/Transform/Allocators.def"
#undef EFD_ALLOCATOR
#undef EFD_ALLOCATOR_SIMPLE
}

void efd::RegisterQbitAllocator(std::string name, alloc::AllocCtorTy ctor) {
    GetRegistry()->registerAllocator(name, ctor);
}

efd::alloc::RetTy efd::CreateQbitAllocator(std::string name, alloc::ArgTy arg) {
    return GetRegistry()->createAllocator(name, arg);
}

// -------------- Allocator Functions -----------------
#define EFD_ALLOCATOR(_Name_, _Class_) \
    efd::alloc::RetTy efd::Create##_Class_(alloc::ArgTy arg) {\
        return _Class_::Create(arg);\
    }
#define EFD_ALLOCATOR_SIMPLE(_Name_, _Finder_, _Solver_) \
    efd::alloc::RetTy efd::Create##_Finder_##With##_Solver_(alloc::ArgTy arg) {\
        auto allocator = SimpleQbitAllocator::Create(arg);\
        allocator->setMapFinder(_Finder_::Create());\
        allocator->setDepSolver(_Solver_::Create());\
        return std::move(allocator);\
    }
#include "enfield/Transform/Allocators.def"
#undef EFD_ALLOCATOR
#undef EFD_ALLOCATOR_SIMPLE

