#include "enfield/Support/BFSPathFinder.h"

#include <algorithm>
#include <queue>
#include <set>
#include <limits>

efd::BFSPathFinder::BFSPathFinder() {
}

std::vector<uint32_t> efd::BFSPathFinder::find(Graph::Ref g, uint32_t u, uint32_t v) {
    const uint32_t ROOT = std::numeric_limits<uint32_t>::max();

    std::vector<uint32_t> parent(g->size(), ROOT);
    std::vector<bool> marked(g->size(), false);

    std::queue<uint32_t> q;
    q.push(u);
    marked[u] = true;

    while (!q.empty()) {
        uint32_t x = q.front();
        q.pop();

        if (x == v) break;

        std::set<uint32_t> &succ = g->succ(x);
        std::set<uint32_t> &pred = g->pred(x);
        for (uint32_t k : succ) {
            if (!marked[k]) {
                q.push(k);
                marked[k] = true;
                parent[k] = x;
            }
        }

        for (uint32_t k : pred) {
            if (!marked[k]) {
                q.push(k);
                marked[k] = true;
                parent[k] = x;
            }
        }
    }

    // Construct path including 'u' and 'v'
    uint32_t x = v;
    std::vector<uint32_t> path;

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
