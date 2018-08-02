#include "enfield/Transform/Allocators/DepSolverQAllocator.h"
#include "enfield/Transform/PassCache.h"

using namespace efd;

DepSolverQAllocator::DepSolverQAllocator(ArchGraph::sRef archGraph) : QbitAllocator(archGraph) {}

StdSolution DepSolverQAllocator::executeAllocation(QModule::Ref qmod) {
    auto depPass = PassCache::Get<DependencyBuilderWrapperPass>(mMod);
    auto depBuilder = depPass->getData();
    auto& deps = depBuilder.getDependencies();

    return solve(deps);
}
