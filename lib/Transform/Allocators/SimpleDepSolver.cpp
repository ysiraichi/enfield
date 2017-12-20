#include "enfield/Transform/Allocators/SimpleDepSolver.h"

efd::SimpleDepSolver::SimpleDepSolver(ArchGraph::sRef agraph) :
    DepSolverQAllocator(agraph) {
}

efd::Solution efd::SimpleDepSolver::solve(DepsSet& deps) {
    auto initial = mMapFinder->find(mArchGraph.get(), deps);
    return mSolBuilder->build(initial, deps, mArchGraph.get());
}

void efd::SimpleDepSolver::setMapFinder(MappingFinder::sRef finder) {
    mMapFinder = finder;
}

void efd::SimpleDepSolver::setSolBuilder(SolutionBuilder::sRef builder) {
    mSolBuilder = builder;
}

efd::SimpleDepSolver::uRef efd::SimpleDepSolver::
Create(ArchGraph::sRef agraph) {
    return uRef(new SimpleDepSolver(agraph));
}
