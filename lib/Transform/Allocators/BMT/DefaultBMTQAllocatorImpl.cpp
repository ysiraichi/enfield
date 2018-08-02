#include "enfield/Transform/Allocators/BMT/DefaultBMTQAllocatorImpl.h"

using namespace efd;
using namespace bmt;

#include <queue>

SeqNCandidateIterator::SeqNCandidateIterator(const Node::Iterator& it, const Node::Iterator& end)
    : mIt(it), mEnd(end) {}

Node::Ref SeqNCandidateIterator::next() {
    Node::Ref node = nullptr;

    if (hasNext()) {
        node = mIt->get();
        ++mIt;
    }

    return node;
}

bool SeqNCandidateIterator::hasNext() {
    return mIt != mEnd;
}

CandidateVector FirstCandidateSelector::select(uint32_t maxCandidates,
                                               const CandidateVector& candidates) {
    uint32_t selectedSize = std::min((uint32_t) candidates.size(), maxCandidates);
    CandidateVector selected(candidates.begin(), candidates.begin() + selectedSize);
    return selected;
}

Vector GeoDistanceSwapCEstimator::distanceFrom(Graph::Ref g, uint32_t u) {
    uint32_t size = g->size();

    Vector distance(size, _undef);
    std::queue<uint32_t> q;
    std::vector<bool> visited(size, false);

    q.push(u);
    visited[u] = true;
    distance[u] = 0;

    while (!q.empty()) {
        uint32_t u = q.front();
        q.pop();

        for (uint32_t v : g->adj(u)) {
            if (!visited[v]) {
                visited[v] = true;
                distance[v] = distance[u] + 1;
                q.push(v);
            }
        }
    }

    return distance;
}

void GeoDistanceSwapCEstimator::fixGraph(Graph::Ref g) {
    uint32_t size = g->size();
    mDist.assign(size, Vector());
    for (uint32_t i = 0; i < size; ++i) {
        mDist[i] = distanceFrom(g, i);
    }
}

uint32_t GeoDistanceSwapCEstimator::estimate(const Mapping& fromM, const Mapping& toM) {
    uint32_t totalDistance = 0;

    for (uint32_t i = 0, e = fromM.size(); i < e; ++i) {
        if (fromM[i] != _undef) {
            totalDistance += mDist[fromM[i]][toM[i]];
        }
    }

    return totalDistance;
}

uint32_t GeoNearestLQPProcessor::getNearest(const Graph::Ref g, uint32_t u, const Assign& assign) {
    std::vector<bool> visited(mPQubits, false);
    std::queue<uint32_t> q;
    q.push(u);
    visited[u] = true;

    while (!q.empty()) {
        uint32_t v = q.front();
        q.pop();

        if (assign[v] == _undef) return v;

        for (uint32_t w : g->adj(v))
            if (!visited[w]) {
                visited[w] = true;
                q.push(w);
            }
    }

    // There is no way we can not find anyone!!
    ERR << "Can't find any vertice connected to v:" << u << "." << std::endl;
    ExitWith(ExitCode::EXIT_unreachable);
}

void GeoNearestLQPProcessor::process(const Graph::Ref g, Mapping& fromM, Mapping& toM) {
    mPQubits = g->size();
    mVQubits = fromM.size();

    auto fromA = GenAssignment(mPQubits, fromM, false);
    auto toA = GenAssignment(mPQubits, toM, false);

    for (uint32_t i = 0; i < mVQubits; ++i) {
        if (toM[i] == _undef && fromM[i] != _undef) {
            if (toA[fromM[i]] == _undef) {
                toM[i] = fromM[i];
            } else {
                toM[i] = getNearest(g, fromM[i], toA);
            }

            toA[toM[i]] = i;
        }
    }
}

Vector BestMSSelector::select(const TIMatrix& mem) {
    Vector selected;

    uint32_t bestCost = _undef;
    uint32_t bestIdx = _undef;
    uint32_t lastLayer = mem.size() - 1;

    for (uint32_t i = 0, e = mem[lastLayer].size(); i < e; ++i) {
        auto info = mem[lastLayer][i];
        if (info.mappingCost + info.swapEstimatedCost < bestCost) {
            bestCost = info.mappingCost + info.swapEstimatedCost;
            bestIdx = i;
        }
    }

    return { bestIdx };
}

