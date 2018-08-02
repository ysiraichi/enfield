#include "enfield/Transform/Allocators/Simple/PathGuidedSolBuilder.h"
#include "enfield/Support/BFSPathFinder.h"
#include "enfield/Support/Stats.h"

#include <map>

static efd::Stat<uint32_t> TotalSwapCost
("TotalSwapCost", "The total cost yielded by swaps.");
static efd::Stat<double> MeanSwapsSize
("MeanSwapsSize", "The mean of swap sequence size.");
static efd::Stat<uint32_t> SerialSwapsCount
("SerialSwapsCount", "The mean of swap sequence size.");

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
    StdSolution solution { initial, StdSolution::OpSequences(deps.size()), 0 };

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

        auto assign = GenAssignment(g->size(), match);
        auto path = mPathFinder->find(g, u, v);

        if (path.size() > 2) {
            for (auto u : path) {
                // In case qubit 'u' has'n been assigned to no one, we just change
                // the initial mapping here (we already filled the 'assign' in line 57)
                if (solution.mInitial[assign[u]] == _undef) {
                    solution.mInitial[assign[u]] = u;
                }

                if (frozen[u]) changeInitialMapping = false;
                frozen[u] = true;
            }

            for (auto i = path.size() - 2; i >= 1; --i) {
                uint32_t u = path[i], v = path[i+1];

                if (g->isReverseEdge(u, v))
                    std::swap(u, v);
    
                uint32_t a = assign[u], b = assign[v];
                ops.second.push_back({ Operation::K_OP_SWAP, a, b });
    
                if (changeInitialMapping) {
                    std::swap(solution.mInitial[a], solution.mInitial[b]);
                }

                std::swap(match[a], match[b]);
                std::swap(assign[u], assign[v]);
            }

            // If this is the first mapping
            if (changeInitialMapping) {
                ops.second.clear();
            } else {
                if (keepStats) {
                    // ------ Stats
                    SerialSwapsCount += 1;
                    MeanSwapsSize += ops.second.size();
                    TotalSwapCost += SwapCost.getVal() * ops.second.size();
                    // --------------------
                }

                solution.mCost += (SwapCost.getVal() * ops.second.size());
            }

            u = match[a], v = match[b];
        }

        if (g->hasEdge(u, v)) {
            ops.second.push_back({ Operation::K_OP_CNOT, a, b });
        } else {
            solution.mCost += RevCost.getVal();
            ops.second.push_back({ Operation::K_OP_REV, a, b });
        }

        frozen[u] = true;
        frozen[v] = true;

        --freq[d];
    }

    if (keepStats && SerialSwapsCount.getVal())
        MeanSwapsSize /= ((double) SerialSwapsCount.getVal());

    return solution;
}

void efd::PathGuidedSolBuilder::setPathFinder(PathFinder::sRef finder) {
    mPathFinder = finder;
}

efd::PathGuidedSolBuilder::uRef efd::PathGuidedSolBuilder::Create() {
    return uRef(new PathGuidedSolBuilder());
}
