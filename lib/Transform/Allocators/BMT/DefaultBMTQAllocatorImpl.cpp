#include "enfield/Transform/Allocators/BMT/DefaultBMTQAllocatorImpl.h"
#include "enfield/Support/BFSCachedDistance.h"

using namespace efd;
using namespace bmt;

#include <queue>

static Opt<uint32_t> MaxMapSeqCandidates
("-bmt-max-mapseq", "Select the best N mapping sequences from phase 2.", 1, false);

// --------------------- SeqNCandidatesGenerator ------------------------
void SeqNCandidatesGenerator::initImpl() {
    mIt = mMod->stmt_begin();
}

bool SeqNCandidatesGenerator::finishedImpl() {
    return mIt == mMod->stmt_end();
}

std::vector<Node::Ref> SeqNCandidatesGenerator::generateImpl() {
    return { mIt->get() };
}

void SeqNCandidatesGenerator::signalProcessed(Node::Ref node) {
    EfdAbortIf(node != mIt->get(),
               "Node `" << node->toString(false) << "` not the one processed. Actual: `"
               << (*mIt)->toString(false) << "`.");
    ++mIt;
}

SeqNCandidatesGenerator::uRef SeqNCandidatesGenerator::Create() {
    return uRef(new SeqNCandidatesGenerator());
}

// --------------------- FirstCandidateSelector ------------------------
MCandidateVector FirstCandidateSelector::select(uint32_t maxCandidates,
                                                const MCandidateVector& candidates) {
    uint32_t selectedSize = std::min((uint32_t) candidates.size(), maxCandidates);
    MCandidateVector selected(candidates.begin(), candidates.begin() + selectedSize);
    return selected;
}

FirstCandidateSelector::uRef FirstCandidateSelector::Create() {
    return uRef(new FirstCandidateSelector());
}

// --------------------- GeoDistanceSwapCEstimator ------------------------
void GeoDistanceSwapCEstimator::initImpl() {
    BFSCachedDistance bfs;
    bfs.init(mG);

    uint32_t mPQubits = mG->size();
    mDist.assign(mPQubits, Vector());

    mDist.assign(mPQubits, Vector(mPQubits, 0));

    for (uint32_t i = 0; i < mPQubits; ++i) {
        for (uint32_t j = i + 1; j < mPQubits; ++j) {
            mDist[i][j] = bfs.get(i, j);
            mDist[j][i] = mDist[i][j];
        }
    }
}

uint32_t GeoDistanceSwapCEstimator::estimateImpl(const Mapping& fromM,
                                                 const Mapping& toM) {
    uint32_t totalDistance = 0;

    for (uint32_t i = 0, e = fromM.size(); i < e; ++i) {
        if (fromM[i] != _undef) {
            totalDistance += mDist[fromM[i]][toM[i]];
        }
    }

    return totalDistance;
}

GeoDistanceSwapCEstimator::uRef GeoDistanceSwapCEstimator::Create() {
    return uRef(new GeoDistanceSwapCEstimator());
}

// --------------------- GeoNearestLQPProcessor ------------------------
void GeoNearestLQPProcessor::initImpl() {
    BFSCachedDistance bfs;
    bfs.init(mG);

    mPQubits = mG->size();
    mDist.assign(mPQubits, Vector(mPQubits, 0));

    for (uint32_t i = 0; i < mPQubits; ++i) {
        for (uint32_t j = i + 1; j < mPQubits; ++j) {
            mDist[i][j] = bfs.get(i, j);
            mDist[j][i] = mDist[i][j];
        }
    }
}

uint32_t GeoNearestLQPProcessor::getNearest(uint32_t u, const InverseMap& inv) {
    uint32_t minV = 0;
    uint32_t minDist = _undef;

    for (uint32_t v = 0; v < mPQubits; ++v) {
        if (inv[v] == _undef && mDist[u][v] < minDist) {
            minDist = mDist[u][v];
            minV = v;
        }
    }

    return minV;
}

void GeoNearestLQPProcessor::processImpl(Mapping& fromM, Mapping& toM) {
    mVQubits = fromM.size();

    auto fromInv = InvertMapping(mPQubits, fromM, false);
    auto toInv = InvertMapping(mPQubits, toM, false);

    for (uint32_t i = 0; i < mVQubits; ++i) {
        if (toM[i] == _undef && fromM[i] != _undef) {
            if (toInv[fromM[i]] == _undef) {
                toM[i] = fromM[i];
            } else {
                toM[i] = getNearest(fromM[i], toInv);
            }

            toInv[toM[i]] = i;
        }
    }
}

GeoNearestLQPProcessor::uRef GeoNearestLQPProcessor::Create() {
    return uRef(new GeoNearestLQPProcessor());
}

// --------------------- BestNMSSelector ------------------------
Vector BestNMSSelector::select(const TIMatrix& mem) {
    typedef std::pair<uint32_t, uint32_t> UIntPair;

    Vector selected;
    uint32_t maxMapSeq = MaxMapSeqCandidates.getVal();
    uint32_t lastLayer = mem.size() - 1;
    std::priority_queue<UIntPair,
                        std::vector<UIntPair>,
                        std::greater<UIntPair>> pQueue;

    for (uint32_t i = 0, e = mem[lastLayer].size(); i < e; ++i) {
        const auto &info = mem[lastLayer][i];
        pQueue.push(std::make_pair(info.mappingCost + info.swapEstimatedCost, i));
    }

    while (!pQueue.empty() && selected.size() < maxMapSeq) {
        selected.push_back(pQueue.top().second);
        pQueue.pop();
    }

    return selected;
}

BestNMSSelector::uRef BestNMSSelector::Create() {
    return uRef(new BestNMSSelector());
}
