#ifndef __EFD_DEFAULT_BMT_QALLOCATOR_IMPL_H__
#define __EFD_DEFAULT_BMT_QALLOCATOR_IMPL_H__

#include "enfield/Transform/Allocators/BoundedMappingTreeQAllocator.h"

namespace efd {
    /// \brief Sequential iterator.
    ///
    /// It follows the order of the instructions given by the iterators.
    class SeqNCandidateIterator : public NodeCandidateIterator {
        private:
            Node::Iterator mIt;

        protected:
            Node::Ref nextImpl() override;
            bool hasNextImpl() override;

        public:
            typedef SeqNCandidateIterator* Ref;
            typedef std::unique_ptr<SeqNCandidateIterator> uRef;

            static uRef Create();
    };

    /// \brief Gets the first \em maxCandidates from \em candidates.
    class FirstCandidateSelector : public CandidateSelector {
        public:
            typedef FirstCandidateSelector* Ref;
            typedef std::unique_ptr<FirstCandidateSelector> uRef;

            bmt::CandidateVector select(uint32_t maxCandidates,
                                        const bmt::CandidateVector& candidates) override;

            static uRef Create();
    };

    /// \brief The estimation is the sum of all distances.
    ///
    /// Distace is the number of edges between the place where a token is,
    /// and the place where it should be.
    class GeoDistanceSwapCEstimator : public SwapCostEstimator {
        private:
            bmt::Matrix mDist;
            bmt::Vector distanceFrom(Graph::Ref g, uint32_t u);

        protected:
            void preprocess() override;
            uint32_t estimateImpl(const Mapping& fromM, const Mapping& toM) override;

        public:
            typedef GeoDistanceSwapCEstimator* Ref;
            typedef std::unique_ptr<GeoDistanceSwapCEstimator> uRef;

            static uRef Create();
    };

    /// \brief Forces \em toM to map all qubits mapped in \em fromM.
    class GeoNearestLQPProcessor : public LiveQubitsPreProcessor {
        private:
            uint32_t mPQubits;
            uint32_t mVQubits;

            uint32_t getNearest(const Graph::Ref g, uint32_t u, const Assign& assign);

        public:
            typedef GeoNearestLQPProcessor* Ref;
            typedef std::unique_ptr<GeoNearestLQPProcessor> uRef;

            void process(const Graph::Ref g, Mapping& fromM, Mapping& toM) override;

            static uRef Create();
    };

    /// \brief Selects the line that yielded the best cost.
    class BestMSSelector : public MapSeqSelector {
        public:
            typedef BestMSSelector* Ref;
            typedef std::unique_ptr<BestMSSelector> uRef;

            bmt::Vector select(const bmt::TIMatrix& mem) override;

            static uRef Create();
    };
}

#endif
