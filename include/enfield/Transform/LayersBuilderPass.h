#ifndef __EFD_LAYERS_BUILDER_PASS_H__
#define __EFD_LAYERS_BUILDER_PASS_H__

#include "enfield/Analysis/Nodes.h"
#include "enfield/Transform/Pass.h"
#include <set>

namespace efd {
    typedef std::set<Node::Ref> Layer;
    typedef std::vector<Layer> Layers;

    /// \brief Build the layers for the program.
    ///
    /// One layer is a set of instructions that, theoreticaly, can be executed
    /// in parallel. i.e. does not need the same qubits.
    ///
    // TODO:
    // Split this class into: building the layer and sorting.
    class LayersBuilderPass : public PassT<Layers> {
        public:
            typedef std::unique_ptr<LayersBuilderPass> uRef;
            typedef LayersBuilderPass* Ref;

            void run(QModule* qmod) override;
            static uRef Create();
    };
}

#endif
