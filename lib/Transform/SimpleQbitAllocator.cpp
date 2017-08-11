#include "enfield/Transform/SimpleQbitAllocator.h"

efd::SimpleQbitAllocator::SimpleQbitAllocator(ArchGraph::sRef agraph) :
    QbitAllocator(agraph) {
}

efd::QbitAllocator::Mapping efd::SimpleQbitAllocator::
solveDependencies(DepsSet& deps) {
    auto initial = mMapFinder->find(mArchGraph, deps);
    mDepSolver->solve(initial, deps, mArchGraph, this);
    return initial;
}

void efd::SimpleQbitAllocator::setMapFinder(MappingFinder::sRef finder) {
    mMapFinder = finder;
}

void efd::SimpleQbitAllocator::setDepSolver(DependenciesSolver::sRef solver) {
    mDepSolver = solver;
}

efd::SimpleQbitAllocator::uRef efd::SimpleQbitAllocator::
Create(ArchGraph::sRef agraph) {
    return uRef(new SimpleQbitAllocator(agraph));
}
