#include "enfield/Transform/Allocators/BMT/ImprovedBMTQAllocatorImpl.h"
#include "enfield/Transform/CircuitGraphBuilderPass.h"
#include "enfield/Transform/PassCache.h"

#include <queue>

using namespace efd;

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

void CircuitCandidatesGenerator::initializeImpl() {
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

