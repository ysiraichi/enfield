#ifndef __EFD_CIRCUIT_GRAPH_BUILDER_PASS_H__
#define __EFD_CIRCUIT_GRAPH_BUILDER_PASS_H__

#include "enfield/Analysis/Nodes.h"
#include "enfield/Transform/Pass.h"
#include "enfield/Transform/QModule.h"
#include "enfield/Transform/CircuitGraph.h"

namespace efd {
    /// \brief Builds the circuit graph corresponding to the \em QModule
    /// given.
    ///
    /// A \em CircuitGraph is a multigraph 'G=(V, E)', where 'V' represents
    /// the qubit operations, and 'E' is a set of sets of edges 'E_1, .., E_n'.
    /// For a qubit 'i', an edge '(u, v)' is in 'E_i' iff 'i' is a qarg for
    /// 'u' and then, for 'v'.
    class CircuitGraphBuilderPass : public PassT<CircuitGraph> {
        public:
            typedef CircuitGraphBuilderPass* Ref;
            typedef std::unique_ptr<CircuitGraphBuilderPass> uRef;

            static uint8_t ID;

            bool run(QModule* qmod) override;
            static uRef Create();
    };
}

#endif
