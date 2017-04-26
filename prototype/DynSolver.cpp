
#include "Analyser.h"
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

struct PermVal {
    int idx;
    vector<int> perm;
};

struct SwapVal {
    int root;
    int cost;
};

vector<int> getPath(Graph &physGraph, int u, int v);
void swapPath(Graph &physGraph, vector<int> &mapping, vector<int> path, ostream &out);
    
string vecToKey(vector<int> &v) {
    string key = "";
    for (int x : v)
        key += std::to_string(x) + "|";
    return key;
}

unordered_map<string, PermVal> genPermutationMap(int phys, int prog) {
    unordered_map<string, PermVal> permMap;

    vector<bool> selector(phys-prog, false);
    while (selector.size() != phys)
        selector.push_back(true);

    int idx = 0;
    do {

        vector<int> perm;
        for (int i = 0; i < phys; ++i)
            if (selector[i])
                perm.push_back(i);

        do {
            permMap.insert(pair<string, PermVal>(vecToKey(perm), { idx++, perm }));
        } while (next_permutation(perm.begin(), perm.end()));

    } while (next_permutation(selector.begin(), selector.end()));

    return permMap;
}

vector<int> genAssign(vector<int> mapping) {
    vector<int> assign(mapping.size(), -1);
    for (int i = 0, e = mapping.size(); i < e; ++i)
        assign[mapping[i]] = i;
    return assign;
}

vector< pair<int, int> > readDependencies(string filename, int &qubits) {
    ifstream ifs(filename.c_str());

    ifs >> qubits;

    vector< pair<int, int> > dependencies;
    for (int u, v; ifs >> u >> v;)
        dependencies.push_back(pair<int, int>(u, v));

    ifs.close();
    return dependencies;
}

vector<int> dynsolve(Graph &physGraph) {
    int qubits;
    vector< pair<int, int> > dependencies = readDependencies(ProgFilename, qubits);

    unordered_map<string, PermVal> permMap = genPermutationMap(physGraph.size(), qubits);

    int permN = permMap.size();
    int depN = dependencies.size();

    cout << "Permutations: " << permN << endl;

    vector< vector<int>* > permIdMap(permN, nullptr);
    for (auto &pair : permMap)
        permIdMap[pair.second.idx] = &pair.second.perm;

    // Map with the minimum number of swaps at time 'i'.
    SwapVal swaps[permN][depN + 1];

    for (int i = 0; i < permN; ++i)
        swaps[i][0] = { i, 0 };

    for (int i = 0; i < permN; ++i)
        for (int j = 1; j <= depN; ++j)
            swaps[i][j] = { -1, UNREACH };

    

    for (int i = 0; i <= depN; ++i) {
        int t = i+1;
        pair<int, int> dep = dependencies[i];

        /*
        cout << "-----------------------------------------------------------" << endl;
        cout << "Dep: " << dep.first << " -> " << dep.second << endl;
        */

        for (auto pair : permMap) {
            int idx = pair.second.idx;
            vector<int> mapping = pair.second.perm;
            // A reverse mapping (Phys -> Prog)
            vector<int> assign = genAssign(mapping);
            
            int sourceIdx = permMap[vecToKey(mapping)].idx;
            int targetIdx = sourceIdx;

            SwapVal *source = &swaps[sourceIdx][t-1];
            SwapVal *target = &swaps[targetIdx][t];

            if (source->cost == UNREACH)
                continue;

            if (source->root == -1)
                cout << "Error at comb:" << sourceIdx << " - t:" << t-1 << endl;

            SwapVal finalVal;
            if (min(source->cost, target->cost) == source->cost)
                finalVal = *source;
            else 
                finalVal = *target;

            int u = mapping[dep.first];
            int v = mapping[dep.second];
            if (!physGraph.hasEdge(u, v)) {
                vector<int> newMapping = mapping;

                vector<int> path = getPath(physGraph, u, v);
                swapPath(physGraph, newMapping, path, cerr);

                targetIdx = permMap[vecToKey(newMapping)].idx;
                target = &swaps[targetIdx][t];

                int newCalcCost = source->cost + ((path.size() - 1) * SwapCost);
                int oldCalcCost = target->cost;

                if (min(newCalcCost, oldCalcCost) == newCalcCost)
                    finalVal = { source->root, newCalcCost };
                else
                    finalVal = *target;
            }

            if (physGraph.isReverseEdge(u, v))
                finalVal.cost += RevCost;

            /*
            cout << "{ map:[ ";
            for (int x : mapping)
                cout << "(" << x << ") ";
            cout << "], cost:" << finalCost <<  " }" << endl;
            */

            *target = finalVal;
        }
    }

    SwapVal minCost = swaps[0][depN];
    for (int i = 1; i < permN; ++i) {
        int minVal = min(minCost.cost, swaps[i][depN].cost);
        minCost = (minVal == minCost.cost) ? minCost : swaps[i][depN];
    }

    return *(permIdMap[minCost.root]);
}
