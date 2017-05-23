#include "enfield/Support/OneRestrictionSwapFinder.h"

#include <cassert>
#include <queue>
#include <set>
#include <limits>

efd::OneRestrictionSwapFinder::OneRestrictionSwapFinder(Graph* g) : SwapFinder(g) {
}

std::vector<unsigned> efd::OneRestrictionSwapFinder::getPath(Rest r) {
    const unsigned ROOT = std::numeric_limits<unsigned>::max();

    unsigned u = r.mFrom;
    unsigned v = r.mTo;

    std::vector<unsigned> parent(mG->size(), ROOT);
    std::vector<bool> marked(mG->size(), false);

    std::queue<unsigned> q;
    q.push(u);
    marked[u] = true;

    while (!q.empty()) {
        unsigned x = q.front();
        q.pop();

        if (x == v) break;

        std::set<unsigned> &succ = mG->succ(x);
        for (unsigned k : succ) {
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
    } while (parent[x] != ROOT);
    path.push_back(x);

    return path;
}

efd::SwapFinder::SwapVector efd::OneRestrictionSwapFinder::generateFromPath(std::vector<unsigned> path) {
    SwapVector vec;
    // if path > 2, no swap is needed.
    if (path.size() > 2) {
        for (unsigned i = 0, e = path.size(); i < e - 1; ++i)
            vec.push_back(Swap { path[i], path[i+1] });
    }

    return vec;
}

efd::SwapFinder::SwapVector efd::OneRestrictionSwapFinder::findSwaps(RestrictionVector restrictions) {
    assert(restrictions.size() <= 1 && "This is a 'OneRestriction' swap finder.");

    Rest r = restrictions[0];
    std::vector<unsigned> path = getPath(r);
    return generateFromPath(path);
}

efd::OneRestrictionSwapFinder* efd::OneRestrictionSwapFinder::Create(Graph* g) {
    return new OneRestrictionSwapFinder(g);
}
