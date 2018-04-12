#ifndef __EFD_LAYERS_BUILDER_PASS_H__
#define __EFD_LAYERS_BUILDER_PASS_H__

#include "enfield/Transform/Pass.h"
#include "enfield/Analysis/Nodes.h"

#include <set>
#include <unordered_map>

namespace efd {
    class QModule;
    typedef std::vector<Node::Ref> Layer;
    typedef std::vector<Layer> Layers;

    /// \brief Create the layers of the 'QModule'.
    class LayersBuilderPass : public PassT<Layers> {
        public:
            typedef std::unique_ptr<LayersBuilderPass> uRef;
            typedef LayersBuilderPass* Ref;

            static uint8_t ID;

            bool run(QModule* qmod) override;

            /// \brief Create an instance of this class.
            static uRef Create();
    };
}

#endif
