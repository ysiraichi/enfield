#ifndef __EFD_BOUNDED_MAPPING_TREE_QALLOCATOR_H__
#define __EFD_BOUNDED_MAPPING_TREE_QALLOCATOR_H__

#include "enfield/Transform/Allocators/StdSolutionQAllocator.h"
#include "enfield/Support/TokenSwapFinder.h"

namespace efd {
    namespace bmt {
        /// \brief Composition of each candidate in phase 1.
        struct Candidate {
            Mapping m;
            uint32_t cost;
        };

        /// \brief Necessary information for getting the combinations in phase 2.
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

        /// \brief Keep track of the sequence of `Mapping`s and its cost.
        struct MappingSeq {
            MappingVector mappingV;
            uint32_t mappingCost;
        };

        typedef std::vector<MappingSeq> MapSeqCollection;
        typedef std::vector<SwapSeq> SwapSeqCollection;

        /// \brief Holds the sequence of `Mapping`s and `Swaps`to be executed.
        struct MappingSwapSequence {
            MappingVector mappingV;
            SwapSeqCollection swapSeqCollection;
            uint32_t cost;
        };

        typedef std::vector<Node::Ref> PPartition;
        typedef std::vector<PPartition> PPartitionCollection;
    }

    /// \brief Interface for getting the next `Node` to be processed in phase 1.
    struct NodeCandidateIterator {
        typedef NodeCandidateIterator* Ref;
        typedef std::unique_ptr<NodeCandidateIterator> uRef;

        NodeCandidateIterator();

        /// \brief Sets the `QModule` to be iterated.
        void setQModule(QModule::Ref qmod);
        /// \brief Returns the next `Node`.
        Node::Ref next();
        /// \brief Returns whether there are still `Node`s to be processed.
        bool hasNext();

        protected:
            QModule::Ref mMod;
            bool isFirst;

            void checkQModuleSet();
            virtual Node::Ref nextImpl() = 0;
            virtual bool hasNextImpl() = 0;
    };

    /// \brief Interface for selecting candidates (if they are greater than
    /// a max) in phase 1.
    struct CandidateSelector {
        typedef CandidateSelector* Ref;
        typedef std::unique_ptr<CandidateSelector> uRef;
        /// \brief Selects \em maxCandidates from \em candidates.
        virtual bmt::CandidateVector select(uint32_t maxCandidates,
                                            const bmt::CandidateVector& candidates) = 0;
    };

    /// \brief Interface for estimating the number of swaps in phase 2.
    struct SwapCostEstimator {
        typedef SwapCostEstimator* Ref;
        typedef std::unique_ptr<SwapCostEstimator> uRef;

        SwapCostEstimator();

        /// \brief Sets the `Graph`.
        void setGraph(Graph::Ref g);
        /// \brief Estimates the number of swaps to go from \em fromM to \toM.
        uint32_t estimate(const Mapping& fromM, const Mapping& toM);

        protected:
            Graph::Ref mG;

            void checkGraphSet();
            virtual void preprocess() = 0;
            virtual uint32_t estimateImpl(const Mapping& fromM,
                                          const Mapping& toM) = 0;
    };

    /// \brief Interface for preparing the `Mapping`s for fixing the Live
    /// Qubits problem.
    struct LiveQubitsPreProcessor {
        typedef LiveQubitsPreProcessor* Ref;
        typedef std::unique_ptr<LiveQubitsPreProcessor> uRef;
        /// \brief Processes `Mapping` \em toM, based on the graph \em g and on the
        /// last `Mapping` \em fromM.
        virtual void process(const Graph::Ref g, Mapping& fromM, Mapping& toM) = 0;
    };

    /// \brief Selects a number of line numbers from the memoized matrix. 
    struct MapSeqSelector {
        typedef MapSeqSelector* Ref;
        typedef std::unique_ptr<MapSeqSelector> uRef;
        /// \brief Returns a vector with the line indexes chosen to be tracebacked.
        virtual bmt::Vector select(const bmt::TIMatrix& mem) = 0;
    };

    /// \brief Subgraph Isomorphism based Qubit Allocator.
    ///
    /// This QAllocator is split into 3 phases:
    ///     1. Partitions the program into a number of smaller programs,
    ///         and find all* subgraph isomorphisms from the graph of that
    ///         program to the coupling graph (architecture);
    ///     2. Dynamic programming that tests all combinations of subgraph
    ///         isomorphisms, while estimating the cost of glueing them
    ///         together;
    ///     3. Reconstructs the selected sequence of subgraph isomorphisms
    ///         into a program.
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
            /// \brief Sets the implementation for iterating the `Node`s in phase 1.
            void setNodeCandidateIterator(NodeCandidateIterator::uRef it);
            /// \brief Sets the implementation for selecting the children in phase 1.
            void setChildrenSelector(CandidateSelector::uRef sel);
            /// \brief Sets the implementation for selecting the partial solutions in phase 1.
            void setPartialSolutionSelector(CandidateSelector::uRef sel);
            /// \brief Sets the implementation for estimating the swap cost in phase 2.
            void setSwapCostEstimator(SwapCostEstimator::uRef est);
            /// \brief Sets the implementation for fixing the Live Qubits problem in phase 2.
            void setLiveQubitsPreProcessor(LiveQubitsPreProcessor::uRef proc);
            /// \brief Sets the implementation for selecting the `Mapping` sequences in phase 2.
            void setMapSeqSelector(MapSeqSelector::uRef sel);
            /// \brief Sets the implementation for finding the swap sequences in phase 3.
            void setTokenSwapFinder(TokenSwapFinder::uRef finder);

            static uRef Create(ArchGraph::sRef ag);
    };
}

#endif
