#ifndef __EFD_IMPROVED_BMT_QALLOCATOR_IMPL_H__
#define __EFD_IMPROVED_BMT_QALLOCATOR_IMPL_H__

#include "enfield/Transform/Allocators/BoundedMappingTreeQAllocator.h"
#include "enfield/Transform/CircuitGraph.h"

#include <set>
#include <unordered_map>
#include <random>

namespace efd {
    class CircuitCandidatesGenerator : public NodeCandidatesGenerator {
        private:
            typedef std::set<uint32_t> AdjSet;
            typedef std::vector<AdjSet> AdjList;
            typedef std::unordered_map<Node::Ref, uint32_t> NodeUIntMap;
            typedef std::unordered_map<Node::Ref, CircuitGraph::CircuitNode::Ref> NodeCNodeMap;

            CircuitGraph mCGraph;
            CircuitGraph::Iterator mIt;
            NodeCNodeMap mNCNMap;
            NodeUIntMap mReached;
            uint32_t mXbitSize;

            void advanceXbitId(uint32_t i);
            void advanceCNode(CircuitGraph::CircuitNode::Ref cnode);

        protected:
            void initializeImpl() override;
            bool finishedImpl() override;
            std::vector<Node::Ref> generateImpl() override;

        public:
            typedef CircuitCandidatesGenerator* Ref;
            typedef std::unique_ptr<CircuitCandidatesGenerator> uRef;

            void signalProcessed(Node::Ref node) override;

            static uRef Create();
    };

    class WeightedRouletteCandidateSelector : public CandidateSelector {
        public:
            typedef WeightedRouletteCandidateSelector* Ref;
            typedef std::unique_ptr<WeightedRouletteCandidateSelector> uRef;

        private:
            std::mt19937 mGen;
            std::uniform_real_distribution<double> mDist;

            WeightedRouletteCandidateSelector();

        public:
            bmt::MCandidateVector select(uint32_t maxCandidates,
                                         const bmt::MCandidateVector& candidates) override;

            static uRef Create();
    };
}

#endif
