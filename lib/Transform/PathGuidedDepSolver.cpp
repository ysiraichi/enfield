#include "enfield/Transform/PathGuidedDepSolver.h"
#include "enfield/Support/BFSPathFinder.h"

void efd::PathGuidedDepSolver::solve(Mapping initial, DepsSet& deps,
        ArchGraph::sRef agraph, QbitAllocator::Ref allocator) {

    if (mPathFinder.get() == nullptr)
        mPathFinder = BFSPathFinder::Create(agraph);

    Mapping match = initial;

    for (auto& dep : deps) {
        Dep d = dep.mDeps[0];

        // Program qubits (a, b)
        unsigned a = d.mFrom, b = d.mTo;

        // Physical qubits (u, v)
        unsigned u = match[a], v = match[b];

        if (agraph->hasEdge(u, v)) continue;
        if (agraph->isReverseEdge(u, v)) {
            TotalCost += RevCost.getVal();
            continue;
        }

        auto assign = allocator->genAssign(match);
        auto path = mPathFinder->find(u, v);

        // It should stop before swapping the 'source' qubit.
        for (auto i = path.size() - 2; i >= 1; --i) {
            unsigned u = path[i], v = path[i+1];

            if (agraph->isReverseEdge(u, v))
                std::swap(u, v);

            unsigned a = assign[u], b = assign[v];

            TotalCost += SwapCost.getVal();
            allocator->insertSwapBefore(dep, a, b);

            std::swap(match[a], match[b]);
            std::swap(assign[u], assign[v]);
        }
    }
}

void efd::PathGuidedDepSolver::setPathFinder(PathFinder::sRef finder) {
    mPathFinder = finder;
}

efd::PathGuidedDepSolver::uRef efd::PathGuidedDepSolver::Create() {
    return uRef(new PathGuidedDepSolver());
}
