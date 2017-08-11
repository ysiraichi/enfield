#include "enfield/Transform/QbitterDepSolver.h"

void efd::QbitterDepSolver::solve(Mapping initial, DepsSet& deps,
        ArchGraph::sRef agraph, QbitAllocator::Ref allocator) {
    auto mapping = initial;
    for (auto& lineDeps : deps) {
        auto dep = lineDeps[0];

        // (u, v) edge in Arch
        unsigned u = mapping[dep.mFrom], v = mapping[dep.mTo];

        if (agraph->hasEdge(u, v))
            continue;

        if (agraph->isReverseEdge(u, v)) {
            TotalCost += RevCost.getVal();
            continue;
        }

        allocator->replaceByLCNOT(lineDeps, u, 2, v);
        TotalCost += LCXCost.getVal();
    }
}

efd::QbitterDepSolver::uRef efd::QbitterDepSolver::Create() {
    return uRef(new QbitterDepSolver());
}
