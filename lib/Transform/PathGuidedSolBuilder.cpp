#include "enfield/Transform/PathGuidedSolBuilder.h"
#include "enfield/Transform/QbitAllocator.h"
#include "enfield/Support/BFSPathFinder.h"

efd::Solution efd::PathGuidedSolBuilder::build
(Mapping initial, DepsSet& deps, ArchGraph::Ref g) {
    if (mPathFinder.get() == nullptr)
        mPathFinder = BFSPathFinder::Create();

    Mapping match = initial;
    Solution solution { initial, Solution::OpSequences(deps.size()), 0 };

    for (unsigned i = 0, e = deps.size(); i < e; ++i) {
        Dep d = deps[i][0];

        auto& ops = solution.mOpSeqs[i];
        ops.first = deps[i].mCallPoint;

        // Program qubits (a, b)
        unsigned a = d.mFrom, b = d.mTo;

        // Physical qubits (u, v)
        unsigned u = match[a], v = match[b];

        auto assign = GenAssignment(g->size(), match);
        auto path = mPathFinder->find(g, u, v);
    
        // It should stop before swapping the 'source' qubit.
        for (auto i = path.size() - 2; i >= 1; --i) {
            unsigned u = path[i], v = path[i+1];
    
            if (g->isReverseEdge(u, v))
                std::swap(u, v);
    
            unsigned a = assign[u], b = assign[v];
    
            solution.mCost += SwapCost.getVal();
            ops.second.push_back({ Operation::K_OP_SWAP, a, b });
    
            std::swap(match[a], match[b]);
            std::swap(assign[u], assign[v]);
        }

        u = match[a], v = match[b];

        if (g->hasEdge(u, v)) {
            ops.second.push_back({ Operation::K_OP_CNOT, a, b });
        } else if (g->isReverseEdge(u, v)) {
            solution.mCost += RevCost.getVal();
            ops.second.push_back({ Operation::K_OP_REV, a, b });
        }
    }

    return solution;
}

void efd::PathGuidedSolBuilder::setPathFinder(PathFinder::sRef finder) {
    mPathFinder = finder;
}

efd::PathGuidedSolBuilder::uRef efd::PathGuidedSolBuilder::Create() {
    return uRef(new PathGuidedSolBuilder());
}
