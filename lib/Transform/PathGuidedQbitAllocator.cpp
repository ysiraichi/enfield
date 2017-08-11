#include "enfield/Transform/PathGuidedQbitAllocator.h"

efd::PathGuidedQbitAllocator::PathGuidedQbitAllocator(ArchGraph::sRef agraph) :
    QbitAllocator(agraph) {
}

void efd::PathGuidedQbitAllocator::solveFromInitial(Mapping initial, DepsSet& deps) {
}

efd::QbitAllocator::Mapping efd::PathGuidedQbitAllocator::
solveDependencies(DepsSet& deps) {
    auto initial = getInitialMapping(deps);
    solveFromInitial(initial, deps);
    return initial;
}

void efd::PathGuidedQbitAllocator::setPathFinder(PathFinder::sRef finder) {
    mPathFinder = finder;
}
