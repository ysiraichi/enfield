#ifndef __EFD_DEFAULT_BMT_QALLOCATOR_IMPL_H__
#define __EFD_DEFAULT_BMT_QALLOCATOR_IMPL_H__

#include "enfield/Transform/Allocators/BoundedMappingTreeQAllocator.h"

namespace efd {
    class SeqNCandidateIterator : public NodeCandidateIterator {
        private:
            Node::Iterator mIt;
            Node::Iterator mEnd;

        public:
            SeqNCandidateIterator(const Node::Iterator& it, const Node::Iterator& end);
            Node::Ref next() override;
            bool hasNext() override;
    };

    class FirstCandidateSelector : public CandidateSelector {
        public:
            bmt::CandidateVector select(uint32_t maxCandidates,
                                        const bmt::CandidateVector& candidates) override;
    };

    class GeoDistanceSwapCEstimator : public SwapCostEstimator {
        private:
            bmt::Matrix mDist;
            bmt::Vector distanceFrom(Graph::Ref g, uint32_t u);

        public:
            void fixGraph(Graph::Ref g) override;
            uint32_t estimate(const Mapping& fromM, const Mapping& toM) override;
    };

    class GeoNearestLQPProcessor : public LiveQubitsPreProcessor {
        private:
            uint32_t mPQubits;
            uint32_t mVQubits;

            uint32_t getNearest(const Graph::Ref g, uint32_t u, const Assign& assign);

        public:
            void process(const Graph::Ref g, Mapping& fromM, Mapping& toM) override;
    };

    class BestMSSelector : public MapSeqSelector {
        public:
            bmt::Vector select(const bmt::TIMatrix& mem) override;
    };
}

#endif
