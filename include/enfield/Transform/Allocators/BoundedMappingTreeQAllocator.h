#ifndef __EFD_BOUNDED_MAPPING_TREE_QALLOCATOR_H__
#define __EFD_BOUNDED_MAPPING_TREE_QALLOCATOR_H__

#include "enfield/Transform/Allocators/QbitAllocator.h"
#include "enfield/Support/TokenSwapFinder.h"

namespace efd {
    namespace bmt {
        struct Candidate {
            Mapping m;
            uint32_t cost;
        };

        struct TracebackInfo {
            Mapping m;
            uint32_t parent;
            uint32_t mappingCost;
            uint32_t swapEstimatedCost;
        };

        typedef std::vector<uint32_t> Vector;
        typedef std::vector<Vector> Matrix;

        typedef std::vector<Candidate> CandidateVector;
        typedef std::vector<CandidateVector> CandidateVCollection;

        typedef std::vector<TracebackInfo> TIVector;
        typedef std::vector<TIVector> TIMatrix;

        typedef std::vector<Mapping> MappingVector;
        typedef std::vector<std::vector<Mapping>> MappingVectorCollection;

        struct MappingSeq {
            MappingVector mappingV;
            uint32_t mappingCost;
        };

        typedef std::vector<MappingSeq> MapSeqCollection;
        typedef std::vector<SwapSeq> SwapSeqCollection;

        struct MappingSwapSequence {
            MappingVector mappingV;
            SwapSeqCollection swapSeqCollection;
            uint32_t cost;
        };

        typedef std::vector<Node::Ref> PPartition;
        typedef std::vector<PPartition> PPartitionCollection;
    }

    struct NodeCandidateIterator {
        typedef NodeCandidateIterator* Ref;
        typedef std::unique_ptr<NodeCandidateIterator> uRef;
        virtual Node::Ref next() = 0;
        virtual bool hasNext() = 0;
    };

    struct CandidateSelector {
        typedef CandidateSelector* Ref;
        typedef std::unique_ptr<CandidateSelector> uRef;
        virtual bmt::CandidateVector select(uint32_t maxCandidates,
                                            const bmt::CandidateVector& candidates) = 0;
    };

    struct SwapCostEstimator {
        typedef SwapCostEstimator* Ref;
        typedef std::unique_ptr<SwapCostEstimator> uRef;
        virtual void fixGraph(Graph::Ref g) = 0;
        virtual uint32_t estimate(const Mapping& fromM, const Mapping& toM) = 0;
    };

    struct LiveQubitsPreProcessor {
        typedef LiveQubitsPreProcessor* Ref;
        typedef std::unique_ptr<LiveQubitsPreProcessor> uRef;
        virtual void process(const Graph::Ref g, Mapping& fromM, Mapping& toM) = 0;
    };

    struct MapSeqSelector {
        typedef MapSeqSelector* Ref;
        typedef std::unique_ptr<MapSeqSelector> uRef;
        virtual bmt::Vector select(const bmt::TIMatrix& mem) = 0;
    };

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

    class BoundedMappingTreeQAllocator : public QbitAllocator {
        public:
            typedef BoundedMappingTreeQAllocator* Ref;
            typedef std::unique_ptr<BoundedMappingTreeQAllocator> uRef;

        protected:
            uint32_t mMaxChildren;
            uint32_t mMaxPartial;
            DependencyBuilder mDBuilder;
            bmt::PPartitionCollection mPP;

            NodeCandidateIterator::uRef mNCIterator;
            CandidateSelector::uRef mChildrenCSelector;
            CandidateSelector::uRef mPartialSolutionCSelector;
            SwapCostEstimator::uRef mCostEstimator;
            LiveQubitsPreProcessor::uRef mLQPProcessor;
            MapSeqSelector::uRef mMSSelector;
            TokenSwapFinder::uRef mTSFinder;

            bmt::CandidateVCollection phase1();
            bmt::MappingSwapSequence phase2(const bmt::CandidateVCollection& collection);
            Solution phase3(const bmt::MappingSwapSequence& mss);

            bmt::CandidateVector extendCandidates(Dep& dep,
                                                  const std::vector<bool>& mapped,
                                                  const bmt::CandidateVector& candidates,
                                                  bool ignoreChildrenLimit);

            bmt::MappingSeq tracebackPath(const bmt::TIMatrix& mem, uint32_t idx);
            SwapSeq getTransformingSwapsFor(const Mapping& fromM, Mapping toM);

            BoundedMappingTreeQAllocator(ArchGraph::sRef ag);
            Solution executeAllocation(QModule::Ref qmod) override;

        public:
            static uRef Create(ArchGraph::sRef ag);
    };
}

#endif
