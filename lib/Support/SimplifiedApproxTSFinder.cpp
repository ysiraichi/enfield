#include "enfield/Support/SimplifiedApproxTSFinder.h"
#include "enfield/Support/WeightedGraph.h"
#include "enfield/Support/Defs.h"

#include <limits>
#include <queue>
#include <stack>
#include <set>
#include <map>

// White, gray and black are the usual dfs guys.
// Silver is for marking when it is already in the stack
// (for iterative versions).
static const uint32_t _white  = 0;
static const uint32_t _silver = 1;
static const uint32_t _gray   = 2;
static const uint32_t _black  = 3;

static std::vector<uint32_t> findCycleDFS(uint32_t src,
                                          std::vector<std::vector<uint32_t>>& adj) {
    std::vector<uint32_t> color(adj.size(), _white);
    std::vector<uint32_t> pi(adj.size(), efd::_undef);
    std::stack<uint32_t> stack;

    stack.push(src);
    color[src] = _silver;

    uint32_t from, to;
    bool cyclefound = false;

    // The color "hierarchy" goes:
    // white -> silver -> gray -> black
    while (!cyclefound && !stack.empty()) {
        uint32_t u = stack.top();
        if (color[u] == _gray) { color[u] = _black; stack.pop(); continue; }
        color[u] = _gray;

        for (auto v : adj[u]) {
            if (color[v] == _white) {
                pi[v] = u;
                color[v] = _silver;
                stack.push(v);
            } else if (color[v] == _gray) {
                from = u; to = v;
                cyclefound = true;
                break;
            }
        }
    }

    std::vector<uint32_t> cycle;

    if (cyclefound) {
        cycle.push_back(from);

        do {
            from = pi[from];
            cycle.push_back(from);
        } while (from != to);
    }

    return cycle;
}

static std::vector<std::vector<uint32_t>>
findGoodVerticesBFS(efd::Graph::Ref graph, uint32_t src) {
    uint32_t size = graph->size();
    const uint32_t inf = std::numeric_limits<uint32_t>::max();
    // List of good vertices used to reach the 'i'-th vertex.
    // We say 'u' is a good vertex of 'v' iff the path 'src -> u -> v' results in the
    // smallest path from 'src' to 'v'.
    std::vector<std::vector<uint8_t>> goodvlist(size, std::vector<uint8_t>(size, false));
    // Distance from the source.
    std::vector<uint32_t> d(size, inf);
    std::queue<uint32_t> q;

    d[src] = 0;
    q.push(src);
    // Complexity: O(E(G) * V(G))
    while (!q.empty()) {
        uint32_t u = q.front();
        q.pop();

        // Complexity: O(Adj(u) * V(G))
        for (auto v : graph->adj(u)) {
            // If it is our first time visiting 'v' or the distance of 'src -> u -> v'
            // is equal the best distance of 'v' ('d[v]'), then 'u' is a good vertex of
            // 'v'.
            // Complexity: O(1)
            if (d[v] == inf) {
                q.push(v);
                d[v] = d[u] + 1;
            }

            if (d[v] == d[u] + 1) {
                goodvlist[v][v] = true;

                // Every good vertex of 'u' is also a good vertex of 'v'.
                // So, goodvlist[v] should be the union of goodvlist[v] and goodvlist[u],
                // since we can have multiple shortest paths reaching 'v'.
                // Complexity: O(V(G))
                for (uint32_t i = 0; i < size; ++i) {
                    goodvlist[v][i] |= goodvlist[u][i];
                }
            }
        }
    }

    std::vector<std::vector<uint32_t>> goodv(size, std::vector<uint32_t>());

    // Complexity: O(V(G) * V(G))
    for (uint32_t u = 0; u < size; ++u) {
        for (auto v : graph->adj(src)) {
            if (goodvlist[u][v])
                goodv[u].push_back(v);
        }
    }

    return goodv;
}

efd::SwapSeq efd::SimplifiedApproxTSFinder::findImpl(const InverseMap& from, const InverseMap& to) {
    auto fromInv = from;
    auto toInv = to;

    uint32_t size = mG->size();
    std::vector<std::vector<uint32_t>> gprime(size, std::vector<uint32_t>());
    std::vector<bool> inplace(size, true);
    SwapSeq swapseq;

    // Constructing the inverse for 'to' -----------------------
    Mapping toMap(size, _undef);
    for (uint32_t i = 0; i < size; ++i) {
        if (toInv[i] != _undef)
            toMap[toInv[i]] = i;
    }
    // ---------------------------------------------------------

    // Initializing data ---------------------------------------
    // 1. Checking which vertices are inplace.
    for (uint32_t i = 0; i < size; ++i)
        if (fromInv[i] == _undef) inplace[i] = true;
        else inplace[i] = fromInv[i] == toInv[i];

    // 2. Constructing the graph with the good neighbors.
    for (uint32_t i = 0; i < size; ++i) {
        if (fromInv[i] != _undef)
            gprime[i] = mMatrix[i][toMap[fromInv[i]]];
    }
    // ---------------------------------------------------------

    // Main Loop -----------------------------------------------
    do {
        std::vector<uint32_t> swappath;

        // 1. Trying to find a 'happy chain'
        for (uint32_t i = 0; i < size; ++i)
            if (!inplace[i]) {
                swappath = findCycleDFS(i, gprime);
                if (!swappath.empty()) break;
            }

        // 2. If we failed, we want a unhappy swap
        if (swappath.empty()) {
            // We search for an edge (u, v), such that 'u' has a label that
            // is out of place, and 'v' has a label in place.
            for (uint32_t u = 0; u < size; ++u) {
                if (!inplace[u]) {
                    bool found = false;

                    for (auto v : gprime[u])
                        if (inplace[v]) {
                            found = true;
                            swappath = { u, v };
                            break;
                        }

                    if (found) break;
                }
            }
        }

        // 3. Swap what we found
        if (!swappath.empty()) {
            for (uint32_t i = 1, e = swappath.size(); i < e; ++i) {
                auto u = swappath[i-1], v = swappath[i];
                swapseq.push_back({ u, v });
                std::swap(fromInv[u], fromInv[v]);
            }

            // Updating those vertices that were swapped.
            // The others neither were magically put into place nor changed 'their mind'
            // about where to go (which are good neighbors).
            for (uint32_t i = 0, e = swappath.size(); i < e; ++i) {
                // Updating vertex u.
                auto u = swappath[i];

                if (fromInv[u] == _undef) {
                    inplace[u] = true;
                    gprime[u].clear();
                    continue;
                }

                if (fromInv[u] == toInv[u]) inplace[u] = true;
                else inplace[u] = false;

                gprime[u] = mMatrix[u][toMap[fromInv[u]]];
            }
        } else {
            break;
        }
    } while (true);
    // ---------------------------------------------------------

    return swapseq;
}

void efd::SimplifiedApproxTSFinder::preprocess() {
    for (uint32_t u = 0; u < mG->size(); ++u) {
        mMatrix.push_back(findGoodVerticesBFS(mG, u));
    }
}

efd::SimplifiedApproxTSFinder::uRef efd::SimplifiedApproxTSFinder::Create() {
    return uRef(new SimplifiedApproxTSFinder());
}
