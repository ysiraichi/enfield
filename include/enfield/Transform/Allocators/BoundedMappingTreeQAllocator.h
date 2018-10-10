#ifndef __EFD_BOUNDED_MAPPING_TREE_QALLOCATOR_H__
#define __EFD_BOUNDED_MAPPING_TREE_QALLOCATOR_H__

#include "enfield/Transform/Allocators/QbitAllocator.h"
#include "enfield/Transform/XbitToNumberPass.h"
#include "enfield/Support/TokenSwapFinder.h"

#include <queue>

namespace efd {
    namespace bmt {
        /// \brief Used for ordering the nodes based on some weight.
        struct NodeCandidate {
            uint32_t mWeight;
            Node::Ref mNode;
            Dependencies mDeps;
        };

        typedef std::priority_queue<NodeCandidate,
                                    std::vector<NodeCandidate>,
                                    std::greater<NodeCandidate>> NCPQueue;

        /// \brief `LessThan` operator that orders `NodeCandidate`s.
        ///
        /// It compares the weights and the `Node` pointer only, since the
        /// `mDeps` depends on it.
        bool operator>(const NodeCandidate& lhs, const NodeCandidate& rhs);

        /// \brief Composition of each candidate in phase 1.
        struct MappingCandidate {
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

        typedef std::vector<MappingCandidate> MCandidateVector;
        typedef std::vector<MCandidateVector> MCandidateVCollection;

        typedef std::vector<TracebackInfo> TIVector;
        typedef std::vector<TIVector> TIMatrix;

        typedef std::vector<Mapping> MappingVector;
        typedef std::vector<std::vector<Mapping>> MappingVectorCollection;

        /// \brief Keep track of the sequence of `Mapping`s and its cost.
        struct MappingSeq {
            MappingVector mappingV;
            uint32_t mappingCost;
        };

        typedef std::vector<SwapSeq> SwapSeqVector;

        /// \brief Holds the sequence of `Mapping`s and `Swaps`to be executed.
        struct MappingSwapSequence {
            MappingVector mappingV;
            SwapSeqVector swapSeqCollection;
            uint32_t cost;
        };

        typedef std::vector<Node::Ref> PPartition;
        typedef std::vector<PPartition> PPartitionCollection;
    }

    /// \brief Generates a vector with the `Node`s that can be chosen as the
    /// next instruction.
    struct NodeCandidatesGenerator {
        typedef NodeCandidatesGenerator* Ref;
        typedef std::unique_ptr<NodeCandidatesGenerator> uRef;

        virtual ~NodeCandidatesGenerator() = default;
        NodeCandidatesGenerator();

        /// \brief Returns the next collection of candidates.
        std::vector<Node::Ref> generate();
        /// \brief Returns whether we have finished processing the nodes.
        bool finished();
        /// \brief Initializes the generator.
        void init(QModule::Ref qmod);
        /// \brief Signals the generator which node has been selected.
        virtual void signalProcessed(Node::Ref node);

        private:
            bool mInitialized;
            void checkInitialized();

        protected:
            QModule::Ref mMod;

            virtual void initImpl();
            virtual bool finishedImpl() = 0;
            virtual std::vector<Node::Ref> generateImpl() = 0;
    };

    /// \brief Interface for selecting candidates (if they are greater than
    /// a max) in phase 1.
    struct CandidateSelector {
        typedef CandidateSelector* Ref;
        typedef std::unique_ptr<CandidateSelector> uRef;
        virtual ~CandidateSelector() = default;
        /// \brief Selects \em maxCandidates from \em candidates.
        virtual bmt::MCandidateVector select(uint32_t maxCandidates,
                                             const bmt::MCandidateVector& candidates) = 0;
    };

    /// \brief Interface for estimating the number of swaps in phase 2.
    struct SwapCostEstimator {
        typedef SwapCostEstimator* Ref;
        typedef std::unique_ptr<SwapCostEstimator> uRef;

        virtual ~SwapCostEstimator() = default;
        SwapCostEstimator();

        /// \brief Initializes the structure with \p g.
        void init(Graph::Ref g);
        /// \brief Estimates the number of swaps to go from \em fromM to \toM.
        uint32_t estimate(const Mapping& fromM, const Mapping& toM);

        protected:
            Graph::Ref mG;

            void checkInitialized();
            virtual void initImpl() = 0;
            virtual uint32_t estimateImpl(const Mapping& fromM,
                                          const Mapping& toM) = 0;
    };

    /// \brief Interface for preparing the `Mapping`s for fixing the Live
    /// Qubits problem.
    struct LiveQubitsPreProcessor {
        typedef LiveQubitsPreProcessor* Ref;
        typedef std::unique_ptr<LiveQubitsPreProcessor> uRef;

        virtual ~LiveQubitsPreProcessor() = default;
        LiveQubitsPreProcessor();

        /// \brief Initializes the structure with \p g.
        void init(Graph::Ref g);
        /// \brief Processes `Mapping` \em toM, based on the graph \em g and on the
        /// last `Mapping` \em fromM.
        void process(Mapping& fromM, Mapping& toM);

        protected:
            Graph::Ref mG;

            void checkInitialized();
            virtual void initImpl() = 0;
            virtual void processImpl(Mapping& fromM, Mapping& toM) = 0;
    };

    /// \brief Selects a number of line numbers from the memoized matrix. 
    struct MapSeqSelector {
        typedef MapSeqSelector* Ref;
        typedef std::unique_ptr<MapSeqSelector> uRef;
        virtual ~MapSeqSelector() = default;
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
    class BoundedMappingTreeQAllocator : public QbitAllocator {
        public:
            typedef BoundedMappingTreeQAllocator* Ref;
            typedef std::unique_ptr<BoundedMappingTreeQAllocator> uRef;

        protected:
            uint32_t mMaxChildren;
            uint32_t mMaxPartial;
            DependencyBuilder mDBuilder;
            XbitToNumber mXtoN;
            bmt::PPartitionCollection mPP;

            NodeCandidatesGenerator::uRef mNCGenerator;
            CandidateSelector::uRef mChildrenCSelector;
            CandidateSelector::uRef mPartialSolutionCSelector;
            SwapCostEstimator::uRef mCostEstimator;
            LiveQubitsPreProcessor::uRef mLQPProcessor;
            MapSeqSelector::uRef mMSSelector;
            TokenSwapFinder::uRef mTSFinder;

        private:
            bmt::MCandidateVCollection phase1();
            bmt::MappingSwapSequence phase2(const bmt::MCandidateVCollection& collection);
            Mapping phase3(QModule::Ref qmod, const bmt::MappingSwapSequence& mss);

            bmt::MCandidateVector extendCandidates(Dep& dep,
                                                   const std::vector<bool>& mapped,
                                                   const bmt::MCandidateVector& candidates,
                                                   bool ignoreChildrenLimit);

            bmt::NCPQueue rankCandidates(const std::vector<Node::Ref>& nodeCandidates,
                                         const std::vector<bool>& mapped,
                                         const std::vector<std::set<uint32_t>>& neighbors);

            bmt::MappingSeq tracebackPath(const bmt::TIMatrix& mem, uint32_t idx);
            SwapSeq getTransformingSwapsFor(const Mapping& fromM, Mapping toM);
            void normalize(bmt::MappingSwapSequence& mss);

            BoundedMappingTreeQAllocator(ArchGraph::sRef ag);
            Mapping allocate(QModule::Ref qmod) override;

        public:
            /// \brief Sets the implementation for iterating the `Node`s in phase 1.
            void setNodeCandidatesGenerator(NodeCandidatesGenerator::uRef gen);
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
