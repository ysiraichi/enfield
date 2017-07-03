#ifndef __EFD_WEIGHTED_PM_FINDER_H__
#define __EFD_WEIGHTED_PM_FINDER_H__

#include "enfield/Support/WeightedGraph.h"
#include "enfield/Support/PartialMatchingFinder.h"

#include <algorithm>
#include <cstdlib>
#include <queue>
#include <iostream>

namespace efd {
    /// \brief Extends the \em PartialMatchingFinder class for weighted graphs.
    ///
    /// This class finds a graph matching M=(Vm, Em) of G=(Vg, Eg) and H=(Vh, Eh)
    /// such that Eh is weighted.
    ///
    /// So, the idea is to try to maximize the value of the edges matched.
    template <typename T>
    class WeightedPMFinder : public PartialMatchingFinder {
        protected:
            Graph& mG;
            WeightedGraph<T>& mH;

            std::vector<bool> matched;
            std::vector<unsigned> matching;
            std::vector<unsigned> gOutDegree;

            WeightedPMFinder(Graph& g, WeightedGraph<T>& h);
            unsigned findBestVertex(unsigned a, unsigned b);
            unsigned getFirstFree();

        public:
            virtual std::vector<unsigned> find() override;

            /// \brief Creates an instance of this class.
            static std::unique_ptr<WeightedPMFinder<T>> Create(Graph& g, WeightedGraph<T>& h);
    };

    /// \brief Helper class to sort an index vector based on a value vector.
    template <typename T>
    struct IndexOrder {
        std::vector<T>& mRef;
        IndexOrder(std::vector<T>& ref) : mRef(ref) {}
        bool operator()(const unsigned& l, const unsigned& r) { return mRef[l] < mRef[r]; }
    };
}

template <typename T>
efd::WeightedPMFinder<T>::WeightedPMFinder(Graph& g, WeightedGraph<T>& h) :
    mG(g), mH(h) {}

template <typename T>
unsigned efd::WeightedPMFinder<T>::findBestVertex(unsigned a, unsigned b) {
    unsigned gSize = mG.size();
    unsigned hSize = mH.size();
    unsigned bOutDegree = mH.outDegree(b);

    std::vector<unsigned> gCandidates;
    if (a == hSize) {
        for (unsigned u = 0; u < gSize; ++u) {
            gCandidates.push_back(u);
        }
    } else {
        auto& succ = mG.succ(matching[a]);
        gCandidates = std::vector<unsigned>(succ.begin(), succ.end());
    }

    int tightestDiff = gSize;
    unsigned tightestDiffId = gSize;
    unsigned tightestPosId = gSize;

    for (unsigned i = 0, e = gCandidates.size(); i < e; ++i) {
        unsigned v = gCandidates[i];

        if (!matched[v]) {
            int ltight = gOutDegree[v] - bOutDegree;
            if (ltight >= 0 && ltight < tightestDiff) {
                tightestPosId = v;
            } else if (abs(ltight) < tightestDiff) {
                tightestDiffId = v;
                tightestDiff = ltight;
            }
        }
    }

    unsigned u;
    if (tightestPosId != gSize) u = tightestPosId;
    else u = tightestDiffId;

    if (u == gSize) {
        // Forget about this vertex.
        return gSize;
    }

    matched[u] = true;
    for (auto v : mG.pred(u)) --gOutDegree[v];
    return u;
}

template <typename T>
unsigned efd::WeightedPMFinder<T>::getFirstFree() {
    unsigned gSize = mG.size();

    for (unsigned u = 0; u < gSize; ++u) {
        if (!matched[u]) return u;
    }
}

template <typename T>
std::vector<unsigned> efd::WeightedPMFinder<T>::find() {
    unsigned hSize = mH.size();
    unsigned gSize = mG.size();

    // Copying the out-degree of each vertex of \p mG.
    gOutDegree.assign(gSize, 0);
    for (unsigned u = 0; u < gSize; ++u) {
        gOutDegree[u] = mG.outDegree(u);
    }

    // Computing the weight of each vertex.
    // It is computed by summing all edges that have the vertex as its source.
    // After that, we sort the indexes of the vertices by their weight.
    std::vector<T> w(hSize, 0);
    for (unsigned a = 0; a < hSize; ++a) {
        for (auto b : mH.succ(a))
            w[a] += mH.getW(a, b);
    }

    std::vector<unsigned> wOrdIdx(hSize, 0);
    for (unsigned i = 0; i < hSize; ++i) wOrdIdx[i] = i;

    IndexOrder<T> order(w);
    std::sort(wOrdIdx.begin(), wOrdIdx.end(), order);

    // Start assigning (mapping) the vertices
    std::vector<unsigned> notMatched;
    matching.assign(hSize, hSize);
    matched.assign(gSize, false);
    for (int i = hSize-1; i >= 0; --i) {
        unsigned a = wOrdIdx[i];

        // Vertex a from mH has not been matched yet.
        if (matching[a] == hSize) {
            std::vector<unsigned> parent(hSize, hSize);
            std::vector<bool> inQueue(hSize, false);
            std::queue<unsigned> q;
            q.push(a);
            inQueue[a] = true;

            // BFS from 'a' matching its successors.
            while (!q.empty()) {
                a = q.front();
                q.pop();

                unsigned u = findBestVertex(parent[a], a);

                if (u == gSize) {
                    notMatched.push_back(u);
                    break;
                } else {
                    matching[a] = u;
                    for (auto b : mH.succ(a)) {
                        if (matching[b] == hSize && !inQueue[b]) {
                            parent[b] = a;

                            q.push(b);
                            inQueue[b] = true;
                        }
                    }
                }
            }
        }
    }

    for (auto a : notMatched) {
        unsigned u = getFirstFree();
        matching[a] = u;
    }

    return matching;
}

template <typename T>
std::unique_ptr<efd::WeightedPMFinder<T>> 
efd::WeightedPMFinder<T>::Create(Graph& g, WeightedGraph<T>& h) {
    return std::unique_ptr<WeightedPMFinder<T>>(new WeightedPMFinder<T>(g, h));
}

#endif
