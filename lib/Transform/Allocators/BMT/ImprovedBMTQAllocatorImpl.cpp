#include "enfield/Transform/Allocators/BMT/ImprovedBMTQAllocatorImpl.h"
#include "enfield/Transform/CircuitGraphBuilderPass.h"
#include "enfield/Transform/PassCache.h"

#include <queue>
#include <random>

using namespace efd;
using namespace bmt;

// --------------------- CircuitCandidatesGenerator ------------------------
void CircuitCandidatesGenerator::advanceXbitId(uint32_t i) {
    mIt.next(i);
    ++mReached[mIt.get(i)];
}

void CircuitCandidatesGenerator::advanceCNode(CircuitGraph::CircuitNode::Ref cnode) {
    for (auto id : cnode->getXbitsId()) {
        advanceXbitId(id);
    }
}

void CircuitCandidatesGenerator::initImpl() {
    mCGraph = PassCache::Get<CircuitGraphBuilderPass>(mMod)->getData();
    mIt = mCGraph.build_iterator();
    mXbitSize = mCGraph.size();

    for (uint32_t i = 0; i < mXbitSize; ++i) {
        advanceXbitId(i);
    }
}

bool CircuitCandidatesGenerator::finishedImpl() {
    for (uint32_t i = 0; i < mXbitSize; ++i)
        if (!mIt[i]->isOutputNode())
            return false;
    return true;
}

std::vector<Node::Ref> CircuitCandidatesGenerator::generateImpl() {
    std::set<Node::Ref> nodeCandidateSet;

    for (uint32_t i = 0; i < mXbitSize; ++i) {
        auto cnode = mIt[i];
        auto node = cnode->node();

        if (mReached[node] == cnode->numberOfXbits()) {
            nodeCandidateSet.insert(node);
            mNCNMap[node] = cnode.get();
        }
    }

    return std::vector<Node::Ref>(nodeCandidateSet.begin(), nodeCandidateSet.end());
}

void CircuitCandidatesGenerator::signalProcessed(Node::Ref node) {
    auto cnode = mNCNMap[node];
    mNCNMap.erase(node);
    mReached.erase(node);
    advanceCNode(cnode);
}

CircuitCandidatesGenerator::uRef CircuitCandidatesGenerator::Create() {
    return uRef(new CircuitCandidatesGenerator());
}

// --------------------- WeightedRouletteCandidateSelector ------------------------
WeightedRouletteCandidateSelector::WeightedRouletteCandidateSelector()
    : mGen((std::random_device())()), mDist(0.0) {}

MCandidateVector WeightedRouletteCandidateSelector::select(uint32_t maxCandidates,
                                                           const MCandidateVector& candidates) {
    uint32_t selectionNumber = std::min(maxCandidates, (uint32_t) candidates.size());
    uint32_t deletedW = 0;
    uint32_t sqSum = 0;
    uint32_t wSum = 0;

    MCandidateVector selected;
    std::vector<uint32_t> weight;
    std::vector<bool> wasSelected(candidates.size(), false);

    if (selectionNumber == (uint32_t) candidates.size())
        return candidates;

    for (const auto& cand : candidates) {
        sqSum += (cand.cost * cand.cost);
    }

    if (sqSum == 0) {
        weight.assign(candidates.size(), 1);
    } else {
        for (const auto& cand : candidates) {
            weight.push_back(sqSum - (cand.cost * cand.cost));
        }
    }

    for (auto w : weight) {
        wSum += w;
    }

    for (uint32_t i = 0; i < selectionNumber; ++i) {
        double r = mDist(mGen);
        double cummulativeProbability = 0;
        uint32_t j = 0;

        while (cummulativeProbability < r && j < weight.size()) {
            if (!wasSelected[j]) {
                cummulativeProbability += (double) weight[j] / ((double) wSum - deletedW);
            }

            ++j;
        }

        --j;
        wSum -= deletedW;
        deletedW = weight[j];

        wasSelected[j] = true;
        selected.push_back(candidates[j]);
    }

    return selected;
}

WeightedRouletteCandidateSelector::uRef WeightedRouletteCandidateSelector::Create() {
    return uRef(new WeightedRouletteCandidateSelector());
}
