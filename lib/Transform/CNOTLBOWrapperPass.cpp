#include "enfield/Transform/CNOTLBOWrapperPass.h"
#include "enfield/Transform/QModule.h"
#include "enfield/Support/RTTI.h"

uint8_t efd::CNOTLBOWrapperPass::ID = 0;

efd::Ordering efd::CNOTLBOWrapperPass::generate(CircuitGraph& graph) {
    auto& layers = mData.layers;

    auto xbitNumber = graph.size();
    auto qubitNumber = graph.getQSize();
    auto cbitNumber = graph.getCSize();

    bool stop, ugate;
    auto marked = std::vector<bool>(xbitNumber, false);
    auto reached = std::unordered_map<Node::Ref, uint32_t>();
    auto order = Ordering();
    auto it = graph.build_iterator();

    std::vector<Xbit> xbits;

    for (uint32_t i = 0; i < qubitNumber; ++i) {
        it.next(i);
        xbits.push_back(Xbit::Q(i));
    }

    for (uint32_t i = 0; i < cbitNumber; ++i) {
        it.next(i + qubitNumber);
        xbits.push_back(Xbit::C(i));
    }

    do {
        stop = true;

        // Emit U-gates that may be executed in parallel.
        // However, we want to schedule the controlled gates only, as they
        // are the only gates that affects qubit allocation.
        do {
            Layer layer;
            ugate = false;

            for (uint32_t i = 0; i < qubitNumber; ++i) {
                auto qubit = xbits[i];

                if (it[qubit]->isGateNode() && it[qubit]->numberOfXbits() == 1) {
                    layer.insert(it[qubit]->node());
                    it.next(qubit);
                    ugate = true;
                }
            }

            if (!layer.empty()) {
                for (auto node : layer)
                    order.push_back(getNodeId(node));
                layers.push_back(layer);
            }
        } while (ugate);

        Layer layer;
        // Reach gates with non-marked xbits and mark them.
        for (uint32_t i = 0; i < xbitNumber; ++i) {
            auto bit = xbits[i];

            if (it[bit]->isGateNode() && !marked[i]) {
                marked[i] = true;

                auto node = it[bit]->node();

                if (reached.find(node) == reached.end())
                    reached[node] = it[bit]->numberOfXbits();
                --reached[node];
            }
        }

        // Advance the xbits' ref and unmark them.
        for (uint32_t i = 0; i < xbitNumber; ++i) {
            auto bit = xbits[i];
            auto node = it[bit]->node();

            if (it[bit]->isGateNode() && !reached[node]) {
                layer.insert(node);
                marked[i] = false;
                it.next(bit);
            }

            // If ref isn't nullptr, it means that there still are
            // operations to emit.
            if (!it[bit]->isOutputNode()) stop = false;
        }


        if (!layer.empty()) {
            for (auto node : layer)
                order.push_back(getNodeId(node));
            layers.push_back(layer);
        }
    } while (!stop);

    return order;
}

efd::CNOTLBOWrapperPass::uRef efd::CNOTLBOWrapperPass::Create() {
    return uRef(new CNOTLBOWrapperPass());
}
