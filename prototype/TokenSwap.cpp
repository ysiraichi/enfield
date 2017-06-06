#include "TokenSwap.h"
#include <unordered_map>
#include <queue>

using namespace std;

static unordered_map<string, PermVal> PermMap;
static vector<vector<pair<int, int>>> swaps;
    
string vecToKey(vector<int> &v) {
    string key = "";
    for (int x : v)
        key += std::to_string(x) + "|";
    return key;
}

void genPermutationMap(int phys, unordered_map<string, PermVal>& permMap) {
    int idx = 0;
    vector<int> perm(phys, 0);
    for (int i = 0; i < phys; ++i) perm[i] = i;

    do {
        permMap.insert(pair<string, PermVal>(vecToKey(perm), { idx++, perm }));
    } while (next_permutation(perm.begin(), perm.end()));
}

PermVal translateToPermVal(vector<int>& source, vector<int>& target) {
    int size = source.size();
    vector<int> realTgt(size, 0);
    vector<int> inTranslator(size, 0);

    for (int i = 0; i < size; ++i) {
        inTranslator[source[i]] = i;
    }

    for (int i = 0; i < size; ++i) {
        realTgt[i] = inTranslator[target[i]];
    }

    string tgtS = vecToKey(realTgt);
    return PermMap[tgtS];
}

// Source and Target are Phys->Prog mapping.
int getNofSwaps(vector<int>& source, vector<int>& target) {
    int id = translateToPermVal(source, target).idx;
    return swaps[id].size();
}

// Returns the swaps of the physical qubits.
vector<pair<int, int>> getSwaps(vector<int>& source, vector<int>& target) {
    int id = translateToPermVal(source, target).idx;
    return swaps[id];
    // return vector<pair<int, int>>();
}

void computeSwaps(Graph& archG) {
    int size = archG.size();
    genPermutationMap(size, PermMap);

    vector<bool> inserted(PermMap.size(), false);
    swaps.assign(PermMap.size(), vector<pair<int, int>>());

    vector<PermVal*> ref(PermMap.size(), nullptr);
    for (auto& pair : PermMap) {
        ref[pair.second.idx] = &pair.second;
    }

    vector<int> curPerm;
    queue<int> q;

    // Initial permutation [0, 1, 2, 3, 4]
    q.push(0);
    inserted[0] = true;
    while (!q.empty()) {
        int pId = q.front();
        q.pop();

        PermVal& val = *ref[pId];

        for (int u = 0; u < size; ++u) {
            for (int v : archG.succ(u)) {
                vector<int> copy = val.perm;
                std::swap(copy[u], copy[v]);

                string copyStr = vecToKey(copy);
                int copyId = PermMap[copyStr].idx;
                if (!inserted[copyId]) {
                    inserted[copyId] = true;
                    swaps[copyId] = swaps[pId];
                    swaps[copyId].push_back(pair<int, int>(u, v));
                    q.push(copyId);
                }
            }
        }
    }
}
