#ifndef __EFD_DEFAULT_BMT_QALLOCATOR_IMPL_H__
#define __EFD_DEFAULT_BMT_QALLOCATOR_IMPL_H__

#include "enfield/Transform/Allocators/BoundedMappingTreeQAllocator.h"

namespace efd {
    /// \brief Sequential generator.
    ///
    /// It follows the order of the instructions given by the iterators,
    /// and generates always a vector with one `Node`.
    class SeqNCandidatesGenerator : public NodeCandidatesGenerator {
        private:
            Node::Iterator mIt;

        protected:
            void initImpl() override;
            bool finishedImpl() override;
            std::vector<Node::Ref> generateImpl() override;

        public:
            typedef SeqNCandidatesGenerator* Ref;
            typedef std::unique_ptr<SeqNCandidatesGenerator> uRef;

            void signalProcessed(Node::Ref node) override;

            static uRef Create();
    };

    /// \brief Gets the first \em maxCandidates from \em candidates.
    class FirstCandidateSelector : public CandidateSelector {
        public:
            typedef FirstCandidateSelector* Ref;
            typedef std::unique_ptr<FirstCandidateSelector> uRef;

            bmt::MCandidateVector select(uint32_t maxCandidates,
                                         const bmt::MCandidateVector& candidates) override;

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
            void initImpl() override;
            uint32_t estimateImpl(const Mapping& fromM, const Mapping& toM) override;

        public:
            typedef GeoDistanceSwapCEstimator* Ref;
            typedef std::unique_ptr<GeoDistanceSwapCEstimator> uRef;

            static uRef Create();
    };

    /// \brief Forces \em toM to map all qubits mapped in \em fromM.
    class GeoNearestLQPProcessor : public LiveQubitsPreProcessor {
        private:
            bmt::Matrix mDist;
            uint32_t mPQubits;
            uint32_t mVQubits;

            uint32_t getNearest(uint32_t u, const InverseMap& inv);

        protected:
            void initImpl() override;
            void processImpl(Mapping& fromM, Mapping& toM) override;

        public:
            typedef GeoNearestLQPProcessor* Ref;
            typedef std::unique_ptr<GeoNearestLQPProcessor> uRef;

            static uRef Create();
    };

    /// \brief Selects the line that yielded the best cost.
    class BestNMSSelector : public MapSeqSelector {
        public:
            typedef BestNMSSelector* Ref;
            typedef std::unique_ptr<BestNMSSelector> uRef;

            bmt::Vector select(const bmt::TIMatrix& mem) override;

            static uRef Create();
    };
}

#endif
