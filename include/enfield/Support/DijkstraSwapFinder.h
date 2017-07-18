#ifndef __EFD_DIJKSTRA_SWAP_FINDER_H__
#define __EFD_DIJKSTRA_SWAP_FINDER_H__

#include "enfield/Support/SwapFinder.h"
#include "enfield/Support/WeightedGraph.h"

#include <queue>
#include <cassert>
#include <limits>

namespace efd {
    /// \brief Dijkstra algorithm for finding a swap sequence in a
    /// weighted graph.
    template <typename T>
    class DijkstraSwapFinder : public SwapFinder {
        public:
            typedef DijkstraSwapFinder* Ref;
            typedef std::unique_ptr<DijkstraSwapFinder> uRef;

        private:
            typename WeightedGraph<T>::sRef mWG;

            DijkstraSwapFinder(typename WeightedGraph<T>::sRef wg);

            inline bool equal(T l, T r);

        public:
            SwapVector findSwaps(RestrictionVector restrictions) override;

            /// \brief Create an instance of this class.
            static uRef Create(typename WeightedGraph<T>::sRef wg);
    };

    template <> bool DijkstraSwapFinder<unsigned>::equal(unsigned l, unsigned r);
    template <> bool DijkstraSwapFinder<double>::equal(double l, double r);
}

template <typename T>
efd::DijkstraSwapFinder<T>::DijkstraSwapFinder(typename WeightedGraph<T>::sRef wg) : mWG(wg) {}

template <typename T>
efd::SwapFinder::SwapVector
efd::DijkstraSwapFinder<T>::findSwaps(RestrictionVector restrictions) {
    assert(restrictions.size() == 1 && "Only one restriction allowed for dijkstra.");

    unsigned size = mWG->size();
    unsigned from = restrictions[0].mFrom;
    unsigned to = restrictions[0].mTo;

    unsigned inf = std::numeric_limits<unsigned>::max();
    unsigned infw = std::numeric_limits<T>::max();

    std::vector<T> dist(inf, size);
    std::vector<unsigned> parent(inf, size);
    std::vector<bool> visited(inf, false);
    std::priority_queue<std::pair<T, unsigned>> q;

    // Dijkstra Algorithm
    dist[from] = 0;
    q.push(std::make_pair(0, from));
    while (!q.empty()) {
        unsigned u = q.front().second;
        q.pop();

        if (visited[u]) continue;
        visited[u] = true;

        for (auto v : mWG->succ(u)) {
            T newW = dist[u];
            if (newW < dist[v]) {
                parent[v] = u;
                q.push(std::make_pair(newW, v));
            }
        }
    }

    // Reconstructing the path
    assert(!equal(dist[to], infw) && "No existing path in this graph.");

    SwapVector sv;

    // If the path is "from -> x -> y -> to", the swap vector will contain
    // "(y, to), (x, y)" in this order. In other words, "to" will go to "x".
    unsigned v = to, u;
    while ((u = parent[v]) != from) {
        sv.push_back({ u, v });
        v = u;
    }

    return sv;
}

template <> bool efd::DijkstraSwapFinder<unsigned>::equal(unsigned l, unsigned r) {
    return l == r;
}

template <> bool efd::DijkstraSwapFinder<double>::equal(double l, double r) {
    double epsilon = 0.0001;
    double diff = l - r;
    return diff >= -epsilon && diff <= epsilon;
}

template <typename T>
typename efd::DijkstraSwapFinder<T>::uRef
efd::DijkstraSwapFinder<T>::Create(typename WeightedGraph<T>::sRef wg) {
    return uRef(new DijkstraSwapFinder<T>(wg));
}

#endif
