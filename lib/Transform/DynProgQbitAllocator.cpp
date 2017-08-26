#include "enfield/Transform/DynProgQbitAllocator.h"
#include "enfield/Support/BFSPathFinder.h"
#include "enfield/Support/CommandLine.h"

#include <unordered_map>
#include <limits>
#include <queue>
#include <iostream>
#include <algorithm>

const unsigned UNREACH = std::numeric_limits<unsigned>::max();

struct Val {
    unsigned pId;
    Val* parent;
    unsigned cost;
};

struct PermVal {
    unsigned idx;
    std::vector<unsigned> perm;
};

static std::unordered_map<std::string, PermVal> PermMap;
static std::vector<std::vector<std::pair<unsigned, unsigned>>> swaps;
    
static std::string VecToKey(std::vector<unsigned> &v) {
    std::string s("");
    for (unsigned u : v)
        s = s + "|" + std::to_string(u);
    return s;
}

static void genPermutationMap(unsigned n) {
    unsigned idx = 0;
    std::vector<unsigned> perm(n, 0);
    for (int i = 0; i < n; ++i) perm[i] = i;

    do {
        PermMap.insert(std::pair<std::string, PermVal>(VecToKey(perm), { idx++, perm }));
    } while (next_permutation(perm.begin(), perm.end()));
}

static PermVal translateToPermVal
(std::vector<unsigned>& source, std::vector<unsigned>& target) {
    int size = source.size();
    std::vector<unsigned> realTgt(size, 0);
    std::vector<unsigned> inTranslator(size, 0);

    for (int i = 0; i < size; ++i) {
        inTranslator[source[i]] = i;
    }

    for (int i = 0; i < size; ++i) {
        realTgt[i] = inTranslator[target[i]];
    }

    std::string tgtS = VecToKey(realTgt);
    return PermMap[tgtS];
}

// Source and Target are Phys->Prog mapping.
static unsigned getSwapNum
(std::vector<unsigned>& source, std::vector<unsigned>& target) {
    unsigned id = translateToPermVal(source, target).idx;
    return swaps[id].size();
}

// Returns the swaps of the physical qubits.
static std::vector<std::pair<unsigned, unsigned>>
getSwaps(std::vector<unsigned>& source, std::vector<unsigned>& target) {
    unsigned id = translateToPermVal(source, target).idx;
    return swaps[id];
    // return vector<pair<int, int>>();
}

// Pre-process the architechture graph, calculating the optimal swaps from every
// permutation.
static void computeSwaps(efd::Graph archG) {
    int size = archG.size();
    genPermutationMap(size);

    std::vector<bool> inserted(PermMap.size(), false);
    swaps.assign(PermMap.size(), std::vector<std::pair<unsigned, unsigned>>());

    std::vector<PermVal*> ref(PermMap.size(), nullptr);
    for (auto& pair : PermMap) {
        ref[pair.second.idx] = &pair.second;
    }

    std::vector<unsigned> curPerm;
    std::queue<unsigned> q;

    // Initial permutation [0, 1, 2, 3, 4]
    q.push(0);
    inserted[0] = true;
    while (!q.empty()) {
        int pId = q.front();
        q.pop();

        PermVal& val = *ref[pId];

        for (unsigned u = 0; u < size; ++u) {
            for (unsigned v : archG.succ(u)) {
                std::vector<unsigned> copy = val.perm;
                std::swap(copy[u], copy[v]);

                std::string key = VecToKey(copy);
                int copyId = PermMap[key].idx;
                if (!inserted[copyId]) {
                    inserted[copyId] = true;
                    swaps[copyId] = swaps[pId];
                    swaps[copyId].push_back(std::pair<unsigned, unsigned>(u, v));
                    q.push(copyId);
                }
            }

            for (unsigned v : archG.pred(u)) {
                std::vector<unsigned> copy = val.perm;
                std::swap(copy[u], copy[v]);

                std::string key = VecToKey(copy);
                int copyId = PermMap[key].idx;
                if (!inserted[copyId]) {
                    inserted[copyId] = true;
                    swaps[copyId] = swaps[pId];
                    swaps[copyId].push_back(std::pair<unsigned, unsigned>(u, v));
                    q.push(copyId);
                }
            }
        }
    }
}

static inline unsigned min(unsigned a, unsigned b) {
    if (a == UNREACH && b == UNREACH)
        return UNREACH;

    if (a == UNREACH) return b;
    if (b == UNREACH) return a;

    return (a < b) ? a : b;
}

static inline Val minVal(Val& a, Val& b) {
    int cost = min(a.cost, b.cost);
    if (cost == a.cost) return a;
    else return b;
}

unsigned efd::DynProgQbitAllocator::getIntermediateV(unsigned u, unsigned v) {
    auto& succ = mArchGraph->succ(u);

    for (auto& w : succ) {
        for (auto& z : mArchGraph->succ(w))
            if (z == v) return w;
        for (auto& z : mArchGraph->pred(w))
            if (z == v) return w;
    }

    return UNREACH;
}

efd::Solution efd::DynProgQbitAllocator::solve(DepsSet& deps) {
    computeSwaps(*mArchGraph);

    unsigned archQ = mArchGraph->size();
    const unsigned SWAP_COST = SwapCost.getVal();
    const unsigned REV_COST = RevCost.getVal();
    const unsigned LCX_COST = LCXCost.getVal();

    int permN = PermMap.size();
    int depN = deps.size();

    auto finder = BFSPathFinder::Create();

    std::vector<std::vector<unsigned>*> permIdMap(permN, nullptr);
    for (auto &pair : PermMap)
        permIdMap[pair.second.idx] = &pair.second.perm;

    // Map with the minimum number of vals at time 'i'.
    Val vals[permN][depN + 1];

    for (unsigned i = 0; i < permN; ++i)
        vals[i][0] = { i, nullptr, 0 };

    for (unsigned i = 0; i < permN; ++i)
        for (unsigned j = 1; j <= depN; ++j)
            vals[i][j] = { i, nullptr, UNREACH };

    for (unsigned i = 1; i <= depN; ++i) {
        assert(deps[i-1].getSize() == 1 &&
                "Trying to allocate qbits to a gate with more than one dependency.");
        efd::Dep dep = deps[i-1].mDeps[0];

        for (unsigned tgt = 0; tgt < permN; ++tgt) {
            // Check if target tgtPermutation has the dependency required.
            auto& tgtPerm = *permIdMap[tgt];
            // Arch qubit interaction (u, v)
            unsigned u = tgtPerm[dep.mFrom], v = tgtPerm[dep.mTo];

            // We don't use this configuration if (u, v) is neither a norma edge
            // nor a reverse edge of the physical graph nor is at a 2-edge distance
            // (u -> w -> v).
            bool hasEdge = mArchGraph->hasEdge(u, v);
            bool isReverse = mArchGraph->isReverseEdge(u, v);
            bool is2Dist = finder->find(mArchGraph.get(), u, v).size() == 3;
            if (!hasEdge && !isReverse && !is2Dist)
                continue;

            Val minimum { tgt, nullptr, UNREACH };

            for (int src = 0; src < permN; ++src) {
                Val& srcVal = vals[src][i-1];
                if (srcVal.cost == UNREACH)
                    continue;

                unsigned finalCost = srcVal.cost;

                if (tgt != src) {
                    auto srcAssign = GenAssignment(archQ, *permIdMap[src]);
                    auto tgtAssign = GenAssignment(archQ, tgtPerm);
                    finalCost += getSwapNum(srcAssign, tgtAssign) * SWAP_COST;
                }

                if (!hasEdge) {
                    // Increase cost if using reverse edge.
                    if (isReverse)
                        finalCost += REV_COST;
                    // Else, increase cost if using long cnot gate.
                    else if (is2Dist)
                        finalCost += LCX_COST;
                }

                Val thisVal { tgt, &srcVal, finalCost };
                minimum = minVal(minimum, thisVal);
            }

            vals[tgt][i] = minimum;
        }
    }

    // Get the minimum cost setup.
    Val* val = &vals[0][depN];
    for (int i = 1; i < permN; ++i) {
        int minCost = min(val->cost, vals[i][depN].cost);
        val = (minCost == val->cost) ? val : &vals[i][depN];
    }

    Solution solution;
    solution.mCost = val->cost;
    solution.mOpSeqs.assign(depN, std::pair<Node::Ref, Solution::OpVector>());

    // Get the target mappings for each dependency (with its id).
    std::vector<std::pair<unsigned, Mapping>> mappings(depN);

    for (int i = depN-1; i >= 0; --i) {
        assert(val->parent != nullptr && "Nullptr reached too soon.");
        mappings[i] = std::make_pair(val->pId, *(permIdMap)[val->pId]);
        val = val->parent;
    }

    if (depN == 0) {
        for (int i = 0; i < archQ; ++i)
            solution.mInitial.push_back(i);
        solution.mCost = 0;
    } else {
        solution.mInitial = mappings[0].second;
        solution.mOpSeqs[0].first = deps[0].mCallPoint;
        for (int i = 1; i < depN; ++i) {
            unsigned srcId = mappings[i-1].first, tgtId = mappings[i].first;

            auto& ops = solution.mOpSeqs[i];
            auto& src = mappings[i-1].second;
            auto& tgt = mappings[i].second;

            if (srcId != tgtId) {
                auto srcAssign = GenAssignment(archQ, src);
                auto tgtAssign = GenAssignment(archQ, tgt);

                auto swaps = getSwaps(srcAssign, tgtAssign);
                for (auto pair : swaps) {
                    unsigned u = pair.first, v = pair.second;

                    if (mArchGraph->isReverseEdge(u, v))
                        std::swap(u, v);

                    ops.second.push_back({ Operation::K_OP_SWAP, srcAssign[u], srcAssign[v] });
                    std::swap(srcAssign[u], srcAssign[v]);
                }
            }

            auto dep = deps[i][0];
            unsigned a = dep.mFrom, b = dep.mTo;
            unsigned u = tgt[a], v = tgt[b];
            auto assign = GenAssignment(mArchGraph->size(), tgt);

            Operation operation;

            if (mArchGraph->hasEdge(u, v))
                operation = { Operation::K_OP_CNOT, a, b };
            else if (mArchGraph->isReverseEdge(u, v))
                operation = { Operation::K_OP_REV, a, b };
            else {
                auto path = finder->find(mArchGraph.get(), u, v);
                assert(path.size() == 3 && "Can't apply a long cnot.");
                operation = { Operation::K_OP_LCNOT, a, b };
                operation.mW = assign[path[1]];
            }

            ops.first = deps[i].mCallPoint;
            ops.second.push_back(operation);
        }
    }

    return solution;
}

efd::DynProgQbitAllocator::DynProgQbitAllocator(ArchGraph::sRef pGraph) 
    : QbitAllocator(pGraph) {
}

efd::DynProgQbitAllocator::uRef efd::DynProgQbitAllocator::Create
(ArchGraph::sRef archGraph) {
    return uRef(new DynProgQbitAllocator(archGraph));
}
