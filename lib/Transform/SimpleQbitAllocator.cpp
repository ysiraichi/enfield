#include "enfield/Transform/SimpleQbitAllocator.h"

efd::SimpleQbitAllocator::SimpleQbitAllocator(ArchGraph::sRef agraph) :
    QbitAllocator(agraph) {
}

efd::Solution efd::SimpleQbitAllocator::solve(DepsSet& deps) {
    auto initial = mMapFinder->find(mArchGraph.get(), deps);
    return mSolBuilder->build(initial, deps, mArchGraph.get());
}

void efd::SimpleQbitAllocator::setMapFinder(MappingFinder::sRef finder) {
    mMapFinder = finder;
}

void efd::SimpleQbitAllocator::setSolBuilder(SolutionBuilder::sRef builder) {
    mSolBuilder = builder;
}

efd::SimpleQbitAllocator::uRef efd::SimpleQbitAllocator::
Create(ArchGraph::sRef agraph) {
    return uRef(new SimpleQbitAllocator(agraph));
}
