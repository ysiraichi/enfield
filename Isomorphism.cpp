
#include "Isomorphism.h"

#include <vector>
#include <climits>
#include <algorithm>

using namespace std;

struct IsoPack {
    int nonMapped;
    int errors;
    vector<int> mapping;
};


vector< vector<bool> > options;
vector<int> sorted;

Graph *gPhys, *gProg;

// Verifing isomorphism from (i, k) and (f(i), f(k)) -> (v, u)
IsoPack tryAssigning(vector<int> mapping) {
    int errors = 0;
    int nonMapped = 0;

    for (int i = 0, e = mapping.size(); i < e; ++i) {
        if (mapping[i] != -1) {
            int v = mapping[i];

            vector<int> &succProg = gProg->succ(i);
            vector<int> &succPhys = gPhys->succ(v);

            for (int j = 0, f = succProg.size(); j < f; ++j) {
                int u = mapping[succProg[j]];
                vector<int>::iterator it = find(succPhys.begin(), succPhys.end(), u);
                if (it == succPhys.end())
                    ++errors;
            }
        } else ++nonMapped;
    }

    return { nonMapped, errors, mapping };
}

bool hasOptions(int k, vector<bool> assigned) {
    for (int i = 0, e = gPhys->size(); i < e; ++i) {
        if (options[k][i] && !assigned[i])
            return true;
    }
    return false;
}

IsoPack backtrack(int k, vector<bool> assigned, vector<int> mapping) {
    if (k == gProg->size())
        return tryAssigning(mapping);

    IsoPack best = { INT_MAX, INT_MAX, vector<int>() };

    if (!hasOptions(k, assigned))
        return backtrack(k+1, assigned, mapping);

    for (int i = 0, e = gPhys->size(); i < e; ++i) {
        if (options[k][i] && !assigned[i]) {
            assigned[i] = true;
            mapping[k] = i;

            IsoPack pack = backtrack(k+1, assigned, mapping);

            if (pack.nonMapped == 0 && pack.errors == 0)
                return pack;

            if (pack.nonMapped < best.nonMapped || (pack.nonMapped == best.nonMapped 
                        && pack.errors < best.errors))
                best = pack;

            assigned[i] = false;
            mapping[k] = -1;
        }
    }

    if (best.errors == INT_MAX) {
        best = tryAssigning(mapping);
    }

    return best;
}

bool sortByOutDegree(int u, int v) {
    return gProg->outDegree(u) >= gProg->outDegree(v);
}

vector<int> findIsomorphism(Graph &gPhysRef, Graph &gProgRef) {
    gPhys = &gPhysRef;
    gProg = &gProgRef;

    // Matrix GxH
    options.assign(gProg->size(), vector<bool>(gPhys->size(), false));

    for (int i = 0, e = gProg->size(); i < e; ++i)
        for (int j = 0, f = gPhys->size(); j < f; ++j)
            if (gPhys->outDegree(j) >= gProg->outDegree(i))
                options[i][j] = true;

    sorted.assign(gProg->size(), 0);
    for (int i = 0, e = gProg->size(); i < e; ++i)
        sorted[i] = i;
    sort(sorted.begin(), sorted.end(), sortByOutDegree);

    vector<bool> assigned(gPhys->size(), false);
    vector<int> mapping(gProg->size(), -1);

    IsoPack pack = backtrack(0, assigned, mapping);

    return pack.mapping;
}
