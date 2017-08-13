#include "enfield/Support/BFSPathFinder.h"

#include <cassert>
#include <algorithm>
#include <queue>
#include <set>
#include <limits>

efd::BFSPathFinder::BFSPathFinder() {
}

std::vector<unsigned> efd::BFSPathFinder::find(Graph::Ref g, unsigned u, unsigned v) {
    const unsigned ROOT = std::numeric_limits<unsigned>::max();

    std::vector<unsigned> parent(g->size(), ROOT);
    std::vector<bool> marked(g->size(), false);

    std::queue<unsigned> q;
    q.push(u);
    marked[u] = true;

    while (!q.empty()) {
        unsigned x = q.front();
        q.pop();

        if (x == v) break;

        std::set<unsigned> &succ = g->succ(x);
        std::set<unsigned> &pred = g->pred(x);
        for (unsigned k : succ) {
            if (!marked[k]) {
                q.push(k);
                marked[k] = true;
                parent[k] = x;
            }
        }

        for (unsigned k : pred) {
            if (!marked[k]) {
                q.push(k);
                marked[k] = true;
                parent[k] = x;
            }
        }
    }

    // Construct path including 'u' and 'v'
    unsigned x = v;
    std::vector<unsigned> path;

    do {
        path.push_back(x);
        x = parent[x];
    } while (x != ROOT);

    std::reverse(path.begin(), path.end());
    return path;
}

efd::BFSPathFinder::uRef efd::BFSPathFinder::Create() {
    return uRef(new BFSPathFinder());
}
