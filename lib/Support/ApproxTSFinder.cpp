#include "enfield/Support/ApproxTSFinder.h"
#include <limits>
#include <queue>
#include <set>

const uint32_t WHITE = 0;
const uint32_t GRAY  = 1;
const uint32_t BLACK = 2;

static bool findCycleDFS(uint32_t u,
                         bool keepadding,
                         std::vector<std::vector<uint32_t>>& adj,
                         std::vector<uint32_t>& color,
                         std::vector<uint32_t>& cycle) {
    color[u] = GRAY;

    for (auto v : adj[u]) {
        if (color[v] == GRAY) {
            // Cycle found!
            keepadding = true;

            // First element is the gray node found.
            cycle.push_back(v);
            cycle.push_back(u);
            return true;
        }
    }

    for (auto v : adj[u]) {
        if (color[v] == WHITE && findCycleDFS(v, keepadding, adj, color, cycle)) {
            // If 'u' is the gray node, we stop adding nodes to the cycle.
            if (cycle[0] == u) keepadding = false;
            // Else, if we should keep adding, we add.
            else if (keepadding) cycle.push_back(u);
            return true;
        }
    }

    return false;
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
    q.push(src);
    while (!q.empty()) {
        uint32_t u = q.front();
        q.pop();

        // Stop when we get to 'tgt', or we reach the distance of 'tgt'.
        if (u == tgt || d[u] >= d[tgt]) continue;

        for (auto v : graph->succ(u)) {
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
            goodvlist[v][u] = true;
        }

        // As it is supposed to be an undirected graph...
        for (auto v : graph->pred(u)) {
            if (d[v] != inf && d[v] < d[u] + 1)
                continue;
            else if (d[v] == inf) {
                q.push(v);
                d[v] = d[u] + 1;
            }

            goodvlist[v] = goodvlist[u];
            goodvlist[v][u] = true;
        }
    }

    std::vector<uint32_t> goodv;
    for (auto v : graph->succ(src))
        if (goodvlist[tgt][v])
            goodv.push_back(v);
    for (auto v : graph->pred(src))
        if (goodvlist[tgt][v])
            goodv.push_back(v);
    
    return goodv;
}

efd::SwapSeq efd::ApproxTSFinder::find(Graph::Ref graph, Assign from, Assign to) {
    uint32_t size = graph->size();
    std::vector<std::vector<uint32_t>> gprime(size, std::vector<uint32_t>());

    std::set<uint32_t> inplace;
    std::set<uint32_t> notinplace;
    for (uint32_t i = 0; i < size; ++i)
        if (from[i] == to[i]) inplace.insert(i);
        else notinplace.insert(i);

    // Constructing the inverse for 'to' -----------------------
    Mapping toinv(size, 0);
    for (uint32_t i = 0; i < size; ++i)
        toinv[to[i]] = i;
    // ---------------------------------------------------------

    SwapSeq swapseq;

    do {
        // Constructing gprime -----------------------
        for (auto i : notinplace)
            // For each vertex 'i' in 'graph', we want to find good vertices
            // from 'i' to the vertex that should hold the label that is
            // currently in 'i' ('from[i]').
            gprime[i] = findGoodVerticesBFS(graph, i, toinv[from[i]]);
        // -------------------------------------------

        std::vector<uint32_t> swappath;

        // Trying to find a 'happy chain' ------------
        for (auto i : notinplace) {
            bool keepadding = false;
            std::vector<uint32_t> color(size, WHITE);
            if (findCycleDFS(i, keepadding, gprime, color, swappath)) break;
        }
        // -------------------------------------------

        // If we failed, we want a unhappy swap ------
        if (swappath.empty()) {
            // We search for an edge (u, v), such that 'u' has a label that
            // is out of place, and 'v' has a label in place.
            for (auto u : notinplace) {
                bool found = false;

                for (auto v : gprime[u])
                    if (inplace.find(v) != inplace.end()) {
                        found = true;
                        swappath = { u, v };
                        break;
                    }

                if (found) break;
            }
        }
        // -------------------------------------------

        // Swap what we found ------------------------
        if (!swappath.empty()) {
            for (uint32_t i = 1, e = swappath.size(); i < e; ++i) {
                auto u = swappath[i-1], v = swappath[i];
                swapseq.push_back({ u, v });
                std::swap(from[u], from[v]);
            }
        } else {
            break;
        }
        // -------------------------------------------
    } while (true);

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
