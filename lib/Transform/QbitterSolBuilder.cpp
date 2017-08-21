#include "enfield/Transform/QbitterSolBuilder.h"

efd::QbitterSolBuilder::Solution efd::QbitterSolBuilder::build
(Mapping initial, DepsSet& deps, ArchGraph::Ref g) {
    auto mapping = initial;
    Solution solution { initial, QbitAllocator::SwapSequences(deps.size()), 0 };

    for (auto& lineDeps : deps) {
        auto dep = lineDeps[0];

        // (u, v) edge in Arch
        unsigned u = mapping[dep.mFrom], v = mapping[dep.mTo];

        if (g->hasEdge(u, v))
            continue;

        if (g->isReverseEdge(u, v)) {
            TotalCost += RevCost.getVal();
            continue;
        }

        solution.mCost += LCXCost.getVal();
    }

    return solution;
}

efd::QbitterSolBuilder::uRef efd::QbitterSolBuilder::Create() {
    return uRef(new QbitterSolBuilder());
}
