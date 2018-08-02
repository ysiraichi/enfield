#ifndef __EFD_BOUNDED_MAPPING_TREE_QALLOCATOR_H__
#define __EFD_BOUNDED_MAPPING_TREE_QALLOCATOR_H__

#include "enfield/Transform/Allocators/StdSolutionQAllocator.h"
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

    class BoundedMappingTreeQAllocator : public StdSolutionQAllocator {
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
            StdSolution phase3(const bmt::MappingSwapSequence& mss);

            bmt::CandidateVector extendCandidates(Dep& dep,
                                                  const std::vector<bool>& mapped,
                                                  const bmt::CandidateVector& candidates,
                                                  bool ignoreChildrenLimit);

            bmt::MappingSeq tracebackPath(const bmt::TIMatrix& mem, uint32_t idx);
            SwapSeq getTransformingSwapsFor(const Mapping& fromM, Mapping toM);

            BoundedMappingTreeQAllocator(ArchGraph::sRef ag);
            StdSolution buildStdSolution(QModule::Ref qmod) override;

        public:
            static uRef Create(ArchGraph::sRef ag);
    };
}

#endif
