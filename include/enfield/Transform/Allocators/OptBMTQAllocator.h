#ifndef __EFD_OPT_BMT_QALLOCATOR_H__
#define __EFD_OPT_BMT_QALLOCATOR_H__

#include "enfield/Transform/Allocators/QbitAllocator.h"
#include "enfield/Transform/XbitToNumberPass.h"
#include "enfield/Support/BFSCachedDistance.h"
#include "enfield/Support/TokenSwapFinder.h"

#include <random>
#include <queue>

namespace efd {
namespace opt_bmt {
    struct MappingCandidate;
    struct MappingSwapSequence;
    struct TracebackInfo;
}
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
    class OptBMTQAllocator : public QbitAllocator {
        public:
            typedef OptBMTQAllocator* Ref;
            typedef std::unique_ptr<OptBMTQAllocator> uRef;

        protected:
            uint32_t mMaxPartial;

            DependencyBuilder mDBuilder;
            XbitToNumber mXtoN;
            BFSCachedDistance mBFSDistance;

            std::vector<std::vector<Node::Ref>> mPP;
            std::vector<std::vector<uint32_t>> mDistance;

            TokenSwapFinder::uRef mTSFinder;

            std::mt19937 mGen;
            std::uniform_real_distribution<double> mDistribution;

        private:
            std::vector<std::vector<opt_bmt::MappingCandidate>>
                phase1(QModule::Ref qmod);
            opt_bmt::MappingSwapSequence
                phase2(const std::vector<std::vector<opt_bmt::MappingCandidate>>& collection);
            Mapping
                phase3(QModule::Ref qmod, const opt_bmt::MappingSwapSequence& mss);

            std::vector<opt_bmt::MappingCandidate>
                extendCandidates(const Dep& dep,
                                 const std::vector<bool>& mapped,
                                 const std::vector<opt_bmt::MappingCandidate>& candidates);

            void setCandidatesWeight(std::vector<opt_bmt::MappingCandidate>& candidates,
                                     Graph& lastPartitionGraph);

            std::vector<opt_bmt::MappingCandidate>
                filterCandidates(const std::vector<opt_bmt::MappingCandidate>& candidates);

            uint32_t getNearest(uint32_t u, const InverseMap& inv);

            void propagateLiveQubits(const Mapping& fromM, Mapping& toM);

            uint32_t estimateSwapCost(const Mapping& fromM, const Mapping& toM);

            std::vector<Mapping>
                tracebackPath(const std::vector<std::vector<opt_bmt::TracebackInfo>>& mem,
                              uint32_t idx);

            SwapSeq getTransformingSwapsFor(const Mapping& fromM, Mapping toM);

            void normalize(opt_bmt::MappingSwapSequence& mss);

            void init(QModule::Ref qmod);

            OptBMTQAllocator(ArchGraph::sRef ag);
            Mapping allocate(QModule::Ref qmod) override;

        public:
            static uRef Create(ArchGraph::sRef ag);
    };
}

#endif
