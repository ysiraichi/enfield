#include "enfield/Transform/Allocators/BMT/ImprovedBMTQAllocator.h"
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

void CircuitCandidatesGenerator::initialize() {
    mCGraph = PassCache::Get<CircuitGraphBuilderPass>(mMod)->getData();
    mDBuilder = PassCache::Get<DependencyBuilderWrapperPass>(mMod)->getData();
    mIt = mCGraph.build_iterator();
    mXbitSize = mCGraph.size();
    mMapped.assign(mCGraph.getQSize(), false);

    for (uint32_t i = 0; i < mXbitSize; ++i) {
        advanceXbitId(i);
    }
}

void CircuitCandidatesGenerator::signalProcessed(Node::Ref node) {
    auto cnode = mNCNMap[node];
    mNCNMap.erase(node);
    advanceCNode(cnode);
}

std::vector<Node::Ref> CircuitCandidatesGenerator::generateImpl() {
    std::set<Node::Ref> nodeCandidateSet;

    for (uint32_t i = 0; i < mXbitSize; ++i) {
        auto cnode = mIt[i];
        auto node = cnode.node();

        if (mReached[node] == cnode->numberOfXbits()) {
            nodeCandidateSet.insert(node);
            mNCNMap[node] = cnode;
        }
    }

    return std::vector<Node::Ref>(nodeCandidateSet.begin(), nodeCandidateSet.end());
}

bool CircuitCandidatesGenerator::finishedImpl() {
    for (uint32_t i = 0; i < mXbitSize; ++i)
        if (!mIt[i].isOutputNode())
            return false;
    return true;
}

CircuitCandidatesGenerator::uRef CircuitCandidatesGenerator::Create() {
    return uRef(new CircuitCandidatesGenerator());
}

