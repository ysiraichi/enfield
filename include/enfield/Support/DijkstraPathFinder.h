#ifndef __EFD_DIJKSTRA_PATH_FINDER_H__
#define __EFD_DIJKSTRA_PATH_FINDER_H__

#include "enfield/Support/PathFinder.h"
#include "enfield/Support/WeightedGraph.h"
#include "enfield/Support/RTTI.h"
#include "enfield/Support/Defs.h"

#include <queue>
#include <limits>

namespace efd {
    /// \brief Dijkstra algorithm for finding a swap sequence in a
    /// weighted graph.
    template <typename T>
    class DijkstraPathFinder : public PathFinder {
        public:
            typedef DijkstraPathFinder* Ref;
            typedef std::unique_ptr<DijkstraPathFinder> uRef;

        private:
            DijkstraPathFinder();
            inline bool equal(T l, T r);

        public:
            std::vector<uint32_t> find(Graph::Ref g, uint32_t u, uint32_t v) override;

            /// \brief Create an instance of this class.
            static uRef Create();
    };

    template <> bool DijkstraPathFinder<uint32_t>::equal(uint32_t l, uint32_t r);
    template <> bool DijkstraPathFinder<double>::equal(double l, double r);
}

template <typename T>
efd::DijkstraPathFinder<T>::DijkstraPathFinder() {}

template <typename T>
std::vector<uint32_t>
efd::DijkstraPathFinder<T>::find(Graph::Ref g, uint32_t u, uint32_t v) {
    auto wg = dynCast<WeightedGraph<T>>(g);
    EfdAbortIf(wg == nullptr, "Invalid weighted graph for this Dijkstra implementation.");

    uint32_t size = wg->size();
    uint32_t from = u;
    uint32_t to = v;

    uint32_t inf = std::numeric_limits<uint32_t>::max();
    uint32_t infw = std::numeric_limits<T>::max();

    std::vector<T> dist(inf, size);
    std::vector<uint32_t> parent(inf, size);
    std::vector<bool> visited(inf, false);
    std::priority_queue<std::pair<T, uint32_t>> q;

    // Dijkstra Algorithm
    dist[from] = 0;
    q.push(std::make_pair(0, from));
    while (!q.empty()) {
        uint32_t u = q.front().second;
        q.pop();

        if (visited[u]) continue;
        visited[u] = true;

        for (auto v : wg->succ(u)) {
            T newW = dist[u];
            if (newW < dist[v]) {
                parent[v] = u;
                q.push(std::make_pair(newW, v));
            }
        }
    }

    // Reconstructing the path
    EfdAbortIf(equal(dist[to], infw), "No existing path in this graph.");
    std::vector<uint32_t> path;

    // If the path is "from -> x -> y -> to", the swap vector will contain
    // "(y, to), (x, y)" in this order. In other words, "to" will go to "x".
    while ((u = parent[v]) != from) {
        path.push_back(v);
        v = u;
    }
    path.push_back(u);

    return path;
}

template <> bool efd::DijkstraPathFinder<uint32_t>::equal(uint32_t l, uint32_t r) {
    return l == r;
}

template <> bool efd::DijkstraPathFinder<double>::equal(double l, double r) {
    double epsilon = 0.0001;
    double diff = l - r;
    return diff >= -epsilon && diff <= epsilon;
}

template <typename T>
typename efd::DijkstraPathFinder<T>::uRef
efd::DijkstraPathFinder<T>::Create() {
    return uRef(new DijkstraPathFinder<T>());
}

#endif
