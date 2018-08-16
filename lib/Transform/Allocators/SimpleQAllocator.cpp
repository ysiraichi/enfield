#include "enfield/Transform/Allocators/SimpleQAllocator.h"
#include "enfield/Transform/PassCache.h"

efd::SimpleQAllocator::SimpleQAllocator(ArchGraph::sRef agraph) :
    StdSolutionQAllocator(agraph) {
}

efd::StdSolution efd::SimpleQAllocator::buildStdSolution(QModule::Ref qmod) {
    auto &deps = PassCache::Get<DependencyBuilderWrapperPass>(qmod)
        ->getData()
        .getDependencies();

    auto initial = mMapFinder->find(mArchGraph.get(), deps);
    return mSolBuilder->build(initial, deps, mArchGraph.get());
}

void efd::SimpleQAllocator::setMapFinder(MappingFinder::sRef finder) {
    mMapFinder = finder;
}

void efd::SimpleQAllocator::setSolBuilder(SolutionBuilder::sRef builder) {
    mSolBuilder = builder;
}

efd::SimpleQAllocator::uRef efd::SimpleQAllocator::
Create(ArchGraph::sRef agraph) {
    return uRef(new SimpleQAllocator(agraph));
}
