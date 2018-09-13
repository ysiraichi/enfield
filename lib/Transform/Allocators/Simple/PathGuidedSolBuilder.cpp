#include "enfield/Transform/Allocators/Simple/PathGuidedSolBuilder.h"
#include "enfield/Support/BFSPathFinder.h"
#include "enfield/Support/Stats.h"

#include <map>

struct DepComp {
    bool operator()(const efd::Dep& lhs, const efd::Dep& rhs) const {
        if (lhs.mFrom != rhs.mFrom)
            return lhs.mFrom < rhs.mFrom;
        return lhs.mTo < rhs.mTo;
    }
};

efd::StdSolution efd::PathGuidedSolBuilder::build(Mapping initial,
                                               DepsVector& deps,
                                               ArchGraph::Ref g) {
    if (mPathFinder.get() == nullptr)
        mPathFinder = BFSPathFinder::Create();

    bool keepStats = get(SolutionBuilderOptions::KeepStats);
    bool improveInitialMapping = get(SolutionBuilderOptions::ImproveInitial);

    Mapping match = initial;
    StdSolution solution { initial, StdSolution::OpSequences(deps.size()) };

    std::map<Dep, uint32_t, DepComp> freq;
    for (uint32_t i = 0, e = deps.size(); i < e; ++i) {
        Dep d = deps[i][0];
        if (freq.find(d) == freq.end())
            freq[d] = 0;
        ++freq[d];
    }

    std::vector<bool> frozen(g->size(), false);
    for (uint32_t i = 0, e = deps.size(); i < e; ++i) {
        bool changeInitialMapping = improveInitialMapping;
        Dep d = deps[i][0];

        auto& ops = solution.mOpSeqs[i];
        ops.first = deps[i].mCallPoint;

        // Program qubits (a, b)
        uint32_t a = d.mFrom, b = d.mTo;

        // Physical qubits (u, v)
        uint32_t u = match[a], v = match[b];

        auto inv = InvertMapping(g->size(), match);
        auto path = mPathFinder->find(g, u, v);

        if (path.size() > 2) {
            for (auto u : path) {
                // In case qubit 'u' has'n been assigned to no one, we just change
                // the initial mapping here (we already filled the 'inv' in line 57)
                if (solution.mInitial[inv[u]] == _undef) {
                    solution.mInitial[inv[u]] = u;
                }

                if (frozen[u]) changeInitialMapping = false;
                frozen[u] = true;
            }

            uint32_t swapCost = 0;

            for (auto i = path.size() - 2; i >= 1; --i) {
                uint32_t u = path[i], v = path[i+1];

                if (!g->hasEdge(u, v))
                    std::swap(u, v);
    
                uint32_t a = inv[u], b = inv[v];
                ops.second.push_back({ Operation::K_OP_SWAP, a, b });
    
                if (changeInitialMapping) {
                    std::swap(solution.mInitial[a], solution.mInitial[b]);
                }

                std::swap(match[a], match[b]);
                std::swap(inv[u], inv[v]);
            }

            // If this is the first mapping
            if (changeInitialMapping) {
                ops.second.clear();
            }

            u = match[a], v = match[b];
        }

        if (g->hasEdge(u, v)) {
            ops.second.push_back({ Operation::K_OP_CNOT, a, b });
        } else {
            ops.second.push_back({ Operation::K_OP_REV, a, b });
        }

        frozen[u] = true;
        frozen[v] = true;

        --freq[d];
    }

    return solution;
}

void efd::PathGuidedSolBuilder::setPathFinder(PathFinder::sRef finder) {
    mPathFinder = finder;
}

efd::PathGuidedSolBuilder::uRef efd::PathGuidedSolBuilder::Create() {
    return uRef(new PathGuidedSolBuilder());
}
