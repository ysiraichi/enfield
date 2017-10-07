#ifndef __EFD_WEIGHTED_PM_FINDER_H__
#define __EFD_WEIGHTED_PM_FINDER_H__

#include "enfield/Support/WeightedGraph.h"
#include "enfield/Support/PartialMatchingFinder.h"
#include "enfield/Support/RTTI.h"

#include <algorithm>
#include <cstdlib>
#include <cassert>
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
        public:
            typedef WeightedPMFinder<T>* Ref;
            typedef std::unique_ptr<WeightedPMFinder<T>> uRef;
            typedef std::shared_ptr<WeightedPMFinder<T>> sRef;

        protected:
            Graph::Ref mG;
            typename WeightedGraph<T>::Ref mH;

            std::vector<bool> matched;
            std::vector<uint32_t> matching;
            std::vector<uint32_t> gOutDegree;

            WeightedPMFinder();
            uint32_t findBestVertex(uint32_t a, uint32_t b);
            uint32_t getFirstFree();

        public:
            virtual std::vector<uint32_t> find(Graph::Ref g, Graph::Ref h) override;

            /// \brief Creates an instance of this class.
            static uRef Create();
    };

    /// \brief Helper class to sort an index vector based on a value vector.
    template <typename T>
    struct IndexOrder {
        std::vector<T>& mRef;
        IndexOrder(std::vector<T>& ref) : mRef(ref) {}
        bool operator()(const uint32_t& l, const uint32_t& r) { return mRef[l] < mRef[r]; }
    };
}

template <typename T>
efd::WeightedPMFinder<T>::WeightedPMFinder() {}

template <typename T>
uint32_t efd::WeightedPMFinder<T>::findBestVertex(uint32_t a, uint32_t b) {
    uint32_t gSize = mG->size();
    uint32_t hSize = mH->size();
    uint32_t bOutDegree = mH->outDegree(b);

    std::vector<uint32_t> gCandidates;
    if (a == hSize) {
        for (uint32_t u = 0; u < gSize; ++u) {
            gCandidates.push_back(u);
        }
    } else {
        auto& succ = mG->succ(matching[a]);
        gCandidates = std::vector<uint32_t>(succ.begin(), succ.end());
    }

    int tightestDiff = gSize;
    uint32_t tightestDiffId = gSize;
    uint32_t tightestPosId = gSize;

    for (uint32_t i = 0, e = gCandidates.size(); i < e; ++i) {
        uint32_t v = gCandidates[i];

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

    uint32_t u;
    if (tightestPosId != gSize) u = tightestPosId;
    else u = tightestDiffId;

    if (u == gSize) {
        // Forget about this vertex.
        return gSize;
    }

    matched[u] = true;
    for (auto v : mG->pred(u)) --gOutDegree[v];
    return u;
}

template <typename T>
uint32_t efd::WeightedPMFinder<T>::getFirstFree() {
    uint32_t gSize = mG->size();

    for (uint32_t u = 0; u < gSize; ++u) {
        if (!matched[u]) return u;
    }

    assert(false && "Not enough qubits?");
}

template <typename T>
std::vector<uint32_t> efd::WeightedPMFinder<T>::find(Graph::Ref g, Graph::Ref h) {
    assert(h->isWeighted() && "Trying to use weighted partial matching on unweighted graph.");

    mG = g;
    mH = dynCast<WeightedGraph<T>>(h);
    assert(mH != nullptr && "Graph 'h' is not of the specified type.");

    uint32_t hSize = mH->size();
    uint32_t gSize = mG->size();

    // Copying the out-degree of each vertex of \p mG->
    gOutDegree.assign(gSize, 0);
    for (uint32_t u = 0; u < gSize; ++u) {
        gOutDegree[u] = mG->outDegree(u);
    }

    // Computing the weight of each vertex.
    // It is computed by summing all edges that have the vertex as its source.
    // After that, we sort the indexes of the vertices by their weight.
    std::vector<T> w(hSize, 0);
    for (uint32_t a = 0; a < hSize; ++a) {
        for (auto b : mH->succ(a))
            w[a] += mH->getW(a, b);
    }

    std::vector<uint32_t> wOrdIdx(hSize, 0);
    for (uint32_t i = 0; i < hSize; ++i) wOrdIdx[i] = i;

    IndexOrder<T> order(w);
    std::sort(wOrdIdx.begin(), wOrdIdx.end(), order);

    // Start assigning (mapping) the vertices
    std::vector<uint32_t> notMatched;
    matching.assign(hSize, hSize);
    matched.assign(gSize, false);
    for (int i = hSize-1; i >= 0; --i) {
        uint32_t a = wOrdIdx[i];

        // Vertex a from mH->has not been matched yet.
        if (matching[a] == hSize) {
            std::vector<uint32_t> parent(hSize, hSize);
            std::vector<bool> inQueue(hSize, false);
            std::queue<uint32_t> q;
            q.push(a);
            inQueue[a] = true;

            // BFS from 'a' matching its successors.
            while (!q.empty()) {
                a = q.front();
                q.pop();

                uint32_t u = findBestVertex(parent[a], a);

                if (u == gSize) {
                    notMatched.push_back(u);
                    break;
                } else {
                    matching[a] = u;
                    for (auto b : mH->succ(a)) {
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
        if (matching[a] == hSize) {
            uint32_t u = getFirstFree();
            matching[a] = u;
        }
    }

    return matching;
}

template <typename T>
typename efd::WeightedPMFinder<T>::uRef
efd::WeightedPMFinder<T>::Create() {
    return uRef(new WeightedPMFinder<T>());
}

#endif
