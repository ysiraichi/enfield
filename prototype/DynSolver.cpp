
#include "Analyser.h"
#include "TokenSwap.h"
#include "DynSolver.h"

#include <fstream>
#include <algorithm>
#include <unordered_map>
#include <queue>
#include <climits>

using namespace std;

#define UNREACH -1

#define min(a, b) \
    ((a) == UNREACH) ? (((b) == UNREACH) ? UNREACH : (b)) : \
    (((b) == UNREACH) ? (a) : ((a) < (b)) ? (a) : (b))


struct Val {
    int pId;
    Val* parent;
    int cost;
};

vector<int> getPath(Graph &physGraph, int u, int v);
void swapPath(Graph &physGraph, vector<int> &mapping, vector<int> path, ostream &out);

Val minVal(Val& a, Val& b) {
    int cost = min(a.cost, b.cost);
    if (cost == a.cost) return a;
    else return b;
}

vector<int> genAssign(vector<int> mapping) {
    int size = mapping.size();
    vector<int> assign(mapping.size(), -1);
    for (int i = 0; i < size; ++i)
        assign[mapping[i]] = i;
    // Mapping the extra physical qubits into valid program qubits.
    for (int i = 0; i < size; ++i)
        if (mapping[i] == -1)
            mapping[i] = size++;
    return assign;
}

MapResult dynsolve(Graph &physGraph) {
    int qubits;
    vector<pair<int, int>> dependencies = readDependencies(ProgFilename, qubits);

    computeSwaps(physGraph);

    unordered_map<string, PermVal> permMap;
    genPermutationMap(physGraph.size(), permMap);

    int permN = permMap.size();
    int depN = dependencies.size();

    cout << "Permutations: " << permN << endl;

    vector< vector<int>* > permIdMap(permN, nullptr);
    for (auto &pair : permMap)
        permIdMap[pair.second.idx] = &pair.second.perm;

    // Map with the minimum number of vals at time 'i'.
    Val vals[permN][depN + 1];

    for (int i = 0; i < permN; ++i)
        vals[i][0] = { i, nullptr, 0 };

    for (int i = 0; i < permN; ++i)
        for (int j = 1; j <= depN; ++j)
            vals[i][j] = { i, nullptr, UNREACH };

    for (int i = 1; i <= depN; ++i) {
        pair<int, int> dep = dependencies[i-1];

        /*
        cout << "---------------------------------------" << endl;
        cout << "from:" << dep.first << " - to:" << dep.second << endl;
        */

        for (int tgt = 0; tgt < permN; ++tgt) {
            // Check if target tgtPermutation has the dependency required.
            auto& tgtPerm = *permIdMap[tgt];
            // Arch qubit interaction (u, v)
            int u = tgtPerm[dep.first], v = tgtPerm[dep.second];

            if (!physGraph.hasEdge(u, v))
                continue;

            Val minimum { tgt, nullptr, UNREACH };

            for (int src = 0; src < permN; ++src) {
                Val& srcVal = vals[src][i-1];
                if (srcVal.cost == UNREACH)
                    continue;

                int finalCost = srcVal.cost;

                if (tgt != src) {
                    auto srcAssign = genAssign(*permIdMap[src]);
                    auto tgtAssign = genAssign(tgtPerm);
                    finalCost += getNofSwaps(srcAssign, tgtAssign) * SwapCost;
                }

                if (physGraph.isReverseEdge(u, v))
                    finalCost += RevCost;

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

    int bestCost = val->cost;

    // Get the dep->swaps mapping.
    int swapId = depN-1;
    vector<SwapVector> swaps(depN, SwapVector());
    while (val->parent != nullptr) {
        int srcId = val->parent->pId, tgtId = val->pId;

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

    return { initial, swaps, bestCost };
}
