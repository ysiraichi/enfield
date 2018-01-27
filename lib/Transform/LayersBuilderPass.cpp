#include "enfield/Transform/LayersBuilderPass.h"
#include "enfield/Transform/CircuitGraphBuilderPass.h"
#include "enfield/Transform/QModule.h"
#include "enfield/Transform/PassCache.h"

using namespace efd;

uint8_t LayersBuilderPass::ID = 0;

bool LayersBuilderPass::run(QModule* qmod) {
    auto cktPass = PassCache::Get<CircuitGraphBuilderPass>(qmod);
    auto ckt = cktPass->getData();

    auto xbits = ckt.size();
    auto& layers = mData;

    std::map<CircuitNode*, uint32_t> reached;
    std::vector<bool> marked(xbits, false);

    bool keepgoing;

    do {
        Layer layer;
        keepgoing = false;

        // Reach gates with non-marked xbits and mark them.
        for (uint32_t i = 0; i < xbits; ++i) {
            auto cnode = ckt[i];

            if (cnode && !marked[i]) {
                marked[i] = true;

                if (reached.find(cnode) == reached.end())
                    reached[cnode] = cnode->qargsid.size() + cnode->cargsid.size();
                --reached[cnode];
            }
        }

        // Advance the xbits' ckt and unmark them.
        for (uint32_t i = 0; i < xbits; ++i) {
            auto cnode = ckt[i];

            if (cnode && !reached[cnode]) {
                layer.insert(cnode->node);
                marked[i] = false;
                ckt[i] = cnode->child[i];
            }

            if (ckt[i] != nullptr)
                keepgoing = true;
        }

        if (!layer.empty())
            layers.push_back(layer);
    } while (keepgoing);

    return false;
}

LayersBuilderPass::uRef LayersBuilderPass::Create() {
    return uRef(new LayersBuilderPass());
}
