#include "enfield/Transform/Allocators/DynprogQAllocator.h"
#include "enfield/Transform/PassCache.h"
#include "enfield/Support/BFSPathFinder.h"
#include "enfield/Support/CommandLine.h"
#include "enfield/Support/ExpTSFinder.h"

#include <unordered_map>
#include <limits>
#include <queue>
#include <iostream>
#include <algorithm>

const uint32_t UNREACH = std::numeric_limits<uint32_t>::max();

struct Val {
    uint32_t pId;
    Val* parent;
    uint32_t cost;
};

struct PermVal {
    uint32_t idx;
    std::vector<uint32_t> perm;
};

static inline uint32_t min(uint32_t a, uint32_t b) {
    if (a == UNREACH && b == UNREACH)
        return UNREACH;

    if (a == UNREACH) return b;
    if (b == UNREACH) return a;

    return (a < b) ? a : b;
}

static inline Val minVal(Val& a, Val& b) {
    uint32_t cost = min(a.cost, b.cost);
    if (cost == a.cost) return a;
    else return b;
}

uint32_t efd::DynprogQAllocator::getIntermediateV(uint32_t u, uint32_t v) {
    auto& succ = mArchGraph->succ(u);

    for (auto& w : succ) {
        for (auto& z : mArchGraph->succ(w))
            if (z == v) return w;
        for (auto& z : mArchGraph->pred(w))
            if (z == v) return w;
    }

    return UNREACH;
}

efd::StdSolution efd::DynprogQAllocator::buildStdSolution(QModule::Ref qmod) {
    auto &deps = PassCache::Get<DependencyBuilderWrapperPass>(qmod)
        ->getData()
        .getDependencies();

    ExpTSFinder tsp;
    tsp.setGraph(mArchGraph.get());

    auto permutations = tsp.mInverseMaps;
    uint32_t archQ = mArchGraph->size();

    uint32_t permN = permutations.size();
    uint32_t depN = deps.size();

    auto finder = BFSPathFinder::Create();

    // std::vector<std::vector<uint32_t>*> permIdMap(permN, nullptr);
    // for (auto &pair : PermMap)
    //     permIdMap[pair.second.idx] = &pair.second.perm;

    // Map with the minimum number of vals at time 'i'.
    Val vals[permN][depN + 1];

    for (uint32_t i = 0; i < permN; ++i)
        vals[i][0] = { i, nullptr, 0 };

    for (uint32_t i = 0; i < permN; ++i)
        for (uint32_t j = 1; j <= depN; ++j)
            vals[i][j] = { i, nullptr, UNREACH };

    for (uint32_t i = 1; i <= depN; ++i) {
        EfdAbortIf(deps[i-1].size() > 1,
                   "Trying to allocate qbits to a gate with more than one dependency."
                   << " Gate: `" << deps[i-1].mCallPoint->toString(false) << "`.");

        efd::Dep dep = deps[i-1].mDeps[0];

        for (uint32_t tgt = 0; tgt < permN; ++tgt) {
            // Check if target tgtPermutation has the dependency required.
            auto& tgtPerm = permutations[tgt];
            // Arch qubit interaction (u, v)
            uint32_t u = tgtPerm[dep.mFrom], v = tgtPerm[dep.mTo];

            // We don't use this configuration if (u, v) is neither a norma edge
            // nor a reverse edge of the physical graph nor is at a 2-edge distance
            // (u -> w -> v).
            bool hasEdge = mArchGraph->hasEdge(u, v) || mArchGraph->hasEdge(v, u);
            auto uvPath = finder->find(mArchGraph.get(), u, v);
            if (!hasEdge && uvPath.size() != 3)
                continue;

            Val minimum { tgt, nullptr, UNREACH };

            for (uint32_t src = 0; src < permN; ++src) {
                Val& srcVal = vals[src][i-1];
                if (srcVal.cost == UNREACH)
                    continue;

                uint32_t finalCost = srcVal.cost;

                if (tgt != src) {
                    auto srcInverseMap = InvertMapping(archQ, permutations[src]);
                    auto tgtInverseMap = InvertMapping(archQ, tgtPerm);
                    auto swaps = tsp.find(srcInverseMap, tgtInverseMap);

                    for (auto& s : swaps) {
                        finalCost += getSwapCost(s.u, s.v);
                    }
                }

                if (!hasEdge) {
                    finalCost += getBridgeCost(u, uvPath[1], v);
                } else {
                    finalCost += getCXCost(u, v);
                }

                Val thisVal { tgt, &srcVal, finalCost };
                minimum = minVal(minimum, thisVal);
            }

            vals[tgt][i] = minimum;
        }
    }

    // Get the minimum cost setup.
    Val* val = &vals[0][depN];
    for (uint32_t i = 1; i < permN; ++i) {
        uint32_t minCost = min(val->cost, vals[i][depN].cost);
        val = (minCost == val->cost) ? val : &vals[i][depN];
    }

    StdSolution solution;
    solution.mOpSeqs.assign(depN, std::pair<Node::Ref, StdSolution::OpVector>());

    // Get the target mappings for each dependency (with its id).
    std::vector<std::pair<uint32_t, Mapping>> mappings(depN);

    for (int i = depN-1; i >= 0; --i) {
        EfdAbortIf(val->parent == nullptr, "Nullptr reached too soon.");
        mappings[i] = std::make_pair(val->pId, permutations[val->pId]);
        val = val->parent;
    }

    if (depN == 0) {
        for (uint32_t i = 0; i < archQ; ++i)
            solution.mInitial.push_back(i);
    } else {
        solution.mInitial = mappings[0].second;
        solution.mOpSeqs[0].first = deps[0].mCallPoint;
        for (uint32_t i = 1; i < depN; ++i) {
            uint32_t srcId = mappings[i-1].first, tgtId = mappings[i].first;

            auto& ops = solution.mOpSeqs[i];
            auto& src = mappings[i-1].second;
            auto& tgt = mappings[i].second;

            if (srcId != tgtId) {
                auto srcInverseMap = InvertMapping(archQ, src);
                auto tgtInverseMap = InvertMapping(archQ, tgt);

                auto swaps = tsp.find(srcInverseMap, tgtInverseMap);
                for (auto swp : swaps) {
                    uint32_t u = swp.u, v = swp.v;

                    if (!mArchGraph->hasEdge(u, v))
                        std::swap(u, v);

                    ops.second.push_back({ Operation::K_OP_SWAP, srcInverseMap[u], srcInverseMap[v] });
                    std::swap(srcInverseMap[u], srcInverseMap[v]);
                }
            }

            auto dep = deps[i][0];
            uint32_t a = dep.mFrom, b = dep.mTo;
            uint32_t u = tgt[a], v = tgt[b];
            auto inv = InvertMapping(mArchGraph->size(), tgt);

            Operation operation;

            if (mArchGraph->hasEdge(u, v))
                operation = { Operation::K_OP_CNOT, a, b };
            else if (mArchGraph->hasEdge(v, u))
                operation = { Operation::K_OP_REV, a, b };
            else {
                auto path = finder->find(mArchGraph.get(), u, v);

                EfdAbortIf(path.size() != 3,
                           "Can't apply a long cnot. Actual path size: `" << path.size() << "`.");

                operation = { Operation::K_OP_LCNOT, a, b };
                operation.mW = inv[path[1]];
            }

            ops.first = deps[i].mCallPoint;
            ops.second.push_back(operation);
        }
    }

    return solution;
}

efd::DynprogQAllocator::DynprogQAllocator(ArchGraph::sRef pGraph) 
    : StdSolutionQAllocator(pGraph) {
}

efd::DynprogQAllocator::uRef efd::DynprogQAllocator::Create
(ArchGraph::sRef archGraph) {
    return uRef(new DynprogQAllocator(archGraph));
}
