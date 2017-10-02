#include "enfield/Transform/CNOTLBOWrapperPass.h"
#include "enfield/Transform/QModule.h"
#include "enfield/Support/RTTI.h"

efd::Ordering efd::CNOTLBOWrapperPass::generate(CircuitGraph& graph) {
    auto& layers = mData.layers;
    auto qubits = graph.size();

    bool stop, ugate;
    auto ref = graph;
    auto marked = std::vector<bool>(qubits, false);
    auto reached = std::unordered_map<Node::Ref, unsigned>();
    auto order = Ordering();

    do {
        stop = true;

        // Emit U-gates that may be executed in parallel.
        // However, we want to schedule the controlled gates only, as they
        // are the only gates that affects qubit allocation.
        do {
            Layer layer;
            ugate = false;

            for (unsigned i = 0; i < qubits; ++i) {
                if (ref[i]->qargsid.size() == 1) {
                    order.push_back(getNodeId(ref[i]->node));
                    ref[i] = ref[i]->child[i];
                    ugate = true;
                }
            }

            if (!layer.empty()) layers.push_back(layer);
        } while (ugate);

        Layer layer;
        // Reach gates with non-marked qubits and mark them.
        for (unsigned i = 0; i < qubits; ++i) {
            if (!marked[i]) {
                marked[i] = true;

                if (reached.find(ref[i]->node) == reached.end())
                    reached[ref[i]->node] = ref[i]->child.size() - 1;
                --reached[ref[i]->node];
            }
        }

        // Advance the qubits' ref and unmark them.
        for (unsigned i = 0; i < qubits; ++i) {
            if (!reached[ref[i]->node]) {
                order.push_back(getNodeId(ref[i]->node));
                layer.insert(ref[i]->node);

                marked[i] = false;
                ref[i] = ref[i]->child[i];
            }

            // If ref isn't nullptr, it means that there still are
            // operations to emit.
            if (ref[i] != nullptr)
                stop = false;
        }

        if (!layer.empty()) layers.push_back(layer);
    } while (!stop);

    return order;
}

efd::CNOTLBOWrapperPass::uRef efd::CNOTLBOWrapperPass::Create() {
    return uRef(new CNOTLBOWrapperPass());
}
