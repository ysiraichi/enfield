#ifndef __EFD_LAYER_BASED_ORDERING_WRAPPER_PASS_H__
#define __EFD_LAYER_BASED_ORDERING_WRAPPER_PASS_H__

#include "enfield/Transform/CircuitGraphBuilderPass.h"
#include <set>
#include <unordered_map>

namespace efd {
    typedef std::set<Node::Ref> Layer;
    typedef std::vector<Layer> Layers;
    typedef std::vector<unsigned> Ordering;

    struct LayerBasedOrdering {
        Ordering ordering;
        Layers layers;
    };

    /// \brief Interface for ordering the program by a specific layer-based
    /// ordering.
    ///
    /// One layer is a set of instructions that, theoreticaly, can be executed
    /// in parallel. i.e. does not need the same qubits.
    class LayerBasedOrderingWrapperPass : public PassT<LayerBasedOrdering> {
        public:
            typedef std::unique_ptr<LayerBasedOrderingWrapperPass> uRef;
            typedef LayerBasedOrderingWrapperPass* Ref;

        protected:
            std::unordered_map<Node::Ref, unsigned> mStmtId;

            unsigned getNodeId(Node::Ref ref);
            virtual Ordering generate(CircuitGraph& graph) = 0;

        public:
            void run(QModule* qmod) override;
    };
}

#endif
