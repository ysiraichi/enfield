#include "enfield/Transform/DynProgQbitAllocator.h"
#include "enfield/Support/CommandLine.h"

#include <unordered_map>
#include <limits>
#include <queue>
#include <iostream>
#include <algorithm>

const unsigned UNREACH = std::numeric_limits<unsigned>::max();

typedef std::vector<std::pair<unsigned, unsigned>> SwapVector;

struct Val {
    unsigned pId;
    Val* parent;
    unsigned cost;
};

struct MapResult {
    std::vector<unsigned> initial;
    std::vector<SwapVector> swaps;
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

MapResult efd::DynProgQbitAllocator::dynsolve(std::vector<Dependencies>& deps) {
    int qubits;
    const unsigned SWAP_COST = SwapCost.getVal();
    const unsigned REV_COST = RevCost.getVal();
    const unsigned LCX_COST = LCXCost.getVal();

    computeSwaps(*mArchGraph);

    int permN = PermMap.size();
    int depN = deps.size();

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

        /*
        cout << "---------------------------------------" << endl;
        cout << "from:" << dep.first << " - to:" << dep.second << endl;
        */

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
            unsigned w = getIntermediateV(u, v);
            if (!hasEdge && !isReverse && w == UNREACH)
                continue;

            Val minimum { tgt, nullptr, UNREACH };

            for (int src = 0; src < permN; ++src) {
                Val& srcVal = vals[src][i-1];
                if (srcVal.cost == UNREACH)
                    continue;

                unsigned finalCost = srcVal.cost;

                if (tgt != src) {
                    auto srcAssign = genAssign(*permIdMap[src]);
                    auto tgtAssign = genAssign(tgtPerm);
                    finalCost += getSwapNum(srcAssign, tgtAssign) * SWAP_COST;
                }

                if (!hasEdge) {
                    // Increase cost if using reverse edge.
                    if (mArchGraph->isReverseEdge(u, v))
                        finalCost += REV_COST;
                    // Else, increase cost if using long cnot gate.
                    else if (w != UNREACH)
                        finalCost += LCX_COST;
                }

                Val thisVal { tgt, &srcVal, finalCost };
                minimum = minVal(minimum, thisVal);
            }

            /*
            Val *srcVal = minimum.parent;
            cout << "{id:" << srcVal->pId << ", cost:" << srcVal->cost << ", perm:[";
            for (int i : *permIdMap[srcVal->pId]) cout << " " << i;
            cout << " ]}";

            cout << " >> ";

            cout << "{id:" << minimum.pId << ", cost:" << minimum.cost << ", perm:[";
            for (int i : *permIdMap[minimum.pId]) cout << " " << i;
            cout << " ]}" << endl;
            */

            vals[tgt][i] = minimum;
        }
    }

    // Get the minimum cost setup.
    Val* val = &vals[0][depN];
    for (int i = 1; i < permN; ++i) {
        int minCost = min(val->cost, vals[i][depN].cost);
        val = (minCost == val->cost) ? val : &vals[i][depN];
    }

    /*
    cout << "Best: {id:" << val->pId << ", cost:" << val->cost << ", perm:[";
    for (int i : *permIdMap[val->pId]) cout << " " << i;
    cout << " }" << endl;
    */

    unsigned bestCost = val->cost;

    // Get the dep->swaps mapping.
    unsigned swapId = depN-1;
    std::vector<SwapVector> swaps(depN, SwapVector());
    while (val->parent != nullptr) {
        unsigned srcId = val->parent->pId, tgtId = val->pId;

        if (srcId != tgtId) {
            auto srcAssign = genAssign(*permIdMap[srcId]);
            auto tgtAssign = genAssign(*permIdMap[tgtId]);
            swaps[swapId] = getSwaps(srcAssign, tgtAssign);
        }

        val = val->parent;
        --swapId;
    }

    auto initial = *permIdMap[val->pId];

    /*
    cout << "Initial: {id:" << val->pId << ", cost:" << val->cost << ", perm:[";
    for (int i : *permIdMap[val->pId]) cout << " " << i;
    cout << " }" << endl;
    */

    TotalCost = bestCost;
    return { initial, swaps, bestCost };
}

efd::QbitAllocator::Mapping efd::DynProgQbitAllocator::solveDependencies(DepsSet& deps) {
    // Map Prog -> Arch
    MapResult result = dynsolve(deps);
    auto mapping = result.initial;
    auto assignMap = genAssign(mapping);

    for (unsigned i = 0, e = result.swaps.size(); i < e; ++i) {
        for (auto pair : result.swaps[i]) {
            // (u, v) are program qubits.
            // At first we issue swaps from program qubits. The allocator will
            // rename them afterwards.
            unsigned u = assignMap[pair.first], v = assignMap[pair.second];
            insertSwapBefore(deps[i], u, v);

            std::swap(assignMap[pair.first], assignMap[pair.second]);
            std::swap(mapping[u], mapping[v]);
        }

        auto dep = deps[i][0];
        unsigned u = mapping[dep.mFrom], v = mapping[dep.mTo];

        // Insert the LCNOT only if there is no better way of doing it.
        if (!mArchGraph->hasEdge(u, v) && !mArchGraph->isReverseEdge(u, v)) {
            unsigned w = getIntermediateV(u, v);
            replaceByLCNOT(deps[i], u, w, v);
        }

    }

    return result.initial;
}

efd::DynProgQbitAllocator::DynProgQbitAllocator(ArchGraph::sRef pGraph) 
    : QbitAllocator(pGraph) {
}

efd::DynProgQbitAllocator::uRef efd::DynProgQbitAllocator::Create
(ArchGraph::sRef archGraph) {
    return uRef(new DynProgQbitAllocator(archGraph));
}
