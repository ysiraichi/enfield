#ifndef __EFD_IMPROVED_BMT_QALLOCATOR_IMPL_H__
#define __EFD_IMPROVED_BMT_QALLOCATOR_IMPL_H__

#include "enfield/Transform/Allocators/BoundedMappingTreeQAllocator.h"
#include "enfield/Transform/CircuitGraph.h"

#include <set>
#include <unordered_map>
#include <random>

namespace efd {
    /// \brief Generates a vector with the nodes that it can reach using
    /// the `CircuitGraph`.
    ///
    /// Starting from the `InputNode`s in the `CircuitGraph`, it returns
    /// the nodes that are reachable from them within one step. After
    /// signalizing the processment of one `Node`, it advances all qubits
    /// and cbits related to that `Node`.
    ///
    /// Almost like a graph traversal, after topological ordering.
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
            void initImpl() override;
            bool finishedImpl() override;
            std::vector<Node::Ref> generateImpl() override;

        public:
            typedef CircuitCandidatesGenerator* Ref;
            typedef std::unique_ptr<CircuitCandidatesGenerator> uRef;

            void signalProcessed(Node::Ref node) override;

            static uRef Create();
    };

    /// \brief Selects the candidates randomly, based on their cost.
    ///
    /// It first calculates the weight $w_i$ of each element $i$:
    /// \f[
    ///     w_i = \sum_{j}{c_j^2} - c_i^2
    /// \f]
    ///
    /// After, it calculates the probability $p_i$ of each element, based
    /// on their weigth:
    /// \f[
    ///     p_i = w_i / \sum_{j}{w_j}
    /// \f]
    ///
    /// Then, from a random number $r$ in [0, 1], we choose the first element $e$
    /// such that $r < \sum_{i = 0}^{e}{p_i}$.
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
