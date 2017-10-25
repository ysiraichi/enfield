#include "enfield/Support/ApproxTSFinder.h"
#include "enfield/Support/BFSPathFinder.h"
#include "enfield/Support/Defs.h"

#include <limits>
#include <queue>
#include <stack>
#include <set>
#include <cassert>

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

static std::vector<uint32_t>
findGoodVerticesBFS(efd::Graph::Ref graph, uint32_t src, uint32_t tgt) {
    uint32_t size = graph->size();
    const uint32_t inf = std::numeric_limits<uint32_t>::max();
    // List of good vertices used to reach the 'i'-th vertex.
    // We say 'u' is a good vertex of 'v' iff the path 'src -> u -> v' results in the
    // smallest path from 'src' to 'v'.
    std::vector<std::vector<bool>> goodvlist(size, std::vector<bool>(size, false));
    // Distance from the source.
    std::vector<uint32_t> d(size, inf);
    std::queue<uint32_t> q;

    d[src] = 0;
    q.push(src);
    while (!q.empty()) {
        uint32_t u = q.front();
        q.pop();

        // Stop when we get to 'tgt', or we reach the distance of 'tgt'.
        if (u == tgt || d[u] >= d[tgt]) continue;

        for (auto v : graph->adj(u)) {
            // If we find a vertex 'v' already visited, but our distance is worse,
            // then 'u' is not a good vertex of 'v'.
            if (d[v] != inf && d[v] < d[u] + 1)
                continue;
            // If it is our first time visiting 'v' or the distance of 'src -> u -> v'
            // is equal the best distance of 'v' ('d[v]'), then 'u' is a good vertex of
            // 'v'.
            else if (d[v] == inf) {
                q.push(v);
                d[v] = d[u] + 1;
            }

            // Every good vertex of 'u' is also a good vertex of 'v'.
            goodvlist[v] = goodvlist[u];
            goodvlist[v][v] = true;
            goodvlist[v][u] = true;
        }
    }

    std::vector<uint32_t> goodv;

    for (auto v : graph->adj(src))
        if (goodvlist[tgt][v])
            goodv.push_back(v);

    return goodv;
}

efd::SwapSeq efd::ApproxTSFinder::find(Graph::Ref graph, Assign from, Assign to) {
    uint32_t size = graph->size();
    std::vector<std::vector<uint32_t>> gprime(size, std::vector<uint32_t>());
    std::vector<bool> inplace(size, false);
    SwapSeq swapseq;

    // Constructing the inverse for 'to' -----------------------
    Mapping toinv(size, 0);
    for (uint32_t i = 0; i < size; ++i)
        toinv[to[i]] = i;
    // ---------------------------------------------------------

    // Initializing data ---------------------------------------
    // 1. Checking which vertices are inplace.
    for (uint32_t i = 0; i < size; ++i)
        if (from[i] == to[i]) inplace[i] = true;
        else inplace[i] = false;

    // 2. Constructing the graph with the good neighbors.
    for (uint32_t i = 0; i < size; ++i)
        if (!inplace[i])
            // For each vertex 'i' in 'graph', we want to find good vertices
            // from 'i' to the vertex that should hold the label that is
            // currently in 'i' ('from[i]').
            gprime[i] = findGoodVerticesBFS(graph, i, toinv[from[i]]);
        else
            gprime[i].clear();
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
                std::swap(from[u], from[v]);
            }

            // Updating those vertices that were swapped.
            // The others neither were magically put into place nor changed 'their mind'
            // about where to go (which are good neighbors).
            for (uint32_t i = 0, e = swappath.size(); i < e; ++i) {
                // Updating vertex u.
                auto u = swappath[i];

                if (from[u] == to[u]) inplace[u] = true;
                else inplace[u] = false;

                if (!inplace[u]) gprime[u] = findGoodVerticesBFS(graph, u, toinv[from[u]]);
                else gprime[u].clear();
            }
        } else {
            break;
        }
    } while (true);
    // ---------------------------------------------------------

    return swapseq;
}

efd::SwapSeq efd::ApproxTSFinder::find(Assign from, Assign to) {
    return find(mG.get(), from, to);
}

efd::ApproxTSFinder::ApproxTSFinder(Graph::sRef graph) : TokenSwapFinder(graph) {
}

efd::ApproxTSFinder::uRef efd::ApproxTSFinder::Create(Graph::sRef graph) {
    return uRef(new ApproxTSFinder(graph));
}
