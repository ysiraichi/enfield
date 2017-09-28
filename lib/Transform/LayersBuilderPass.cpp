#include "enfield/Transform/LayersBuilderPass.h"
#include "enfield/Transform/DependencyBuilderPass.h"
#include "enfield/Transform/QModule.h"
#include "enfield/Support/RTTI.h"

namespace efd {
    struct LNode {
        unsigned reached;
        Node::Ref node;
        std::vector<unsigned> qargsid;
        std::vector<LNode*> child;
    };

    std::vector<LNode*> BuildLayerGraph(std::vector<Node::Ref> program,
            QbitToNumber& qton);
}

std::vector<efd::LNode*> efd::BuildLayerGraph(std::vector<Node::Ref> program,
                                              QbitToNumber& qton) {
    std::vector<LNode*> graph;
    std::vector<LNode*> last;

    graph.assign(qton.getSize(), nullptr);
    last.assign(qton.getSize(), nullptr);

    for (auto node : program) {
        auto qopnode = dynCast<NDQOp>(node);
        auto parent = node;

        if (auto ifstmt = dynCast<NDIfStmt>(node)) {
            qopnode = ifstmt->getQOp();
        }

        auto qargs = qopnode->getQArgs();
        auto newnode = new LNode();

        newnode->node = parent;
        newnode->reached = qargs->getChildNumber();
        newnode->child = std::vector<LNode*>(qton.getSize(), nullptr);

        for (auto& qarg : *qargs) {
            auto qargid = qton.getUId(qarg->toString());
            newnode->qargsid.push_back(qargid);

            if (last[qargid] == nullptr) {
                graph[qargid] = newnode;
            } else {
                last[qargid]->child[qargid] = newnode;
            }

            last[qargid] = newnode;
        }
    }

    return graph;
}

void efd::LayersBuilderPass::run(QModule* qmod) {
    auto qtonpass = QbitToNumberWrapperPass::Create();
    qtonpass->run(qmod);
    auto qton = qtonpass->getData();

    std::vector<Node::Ref> stmts;
    std::unordered_map<Node::Ref, unsigned> stmtsidMap;
    for (auto it = qmod->stmt_begin(), e = qmod->stmt_end(); it != e; ++it) {
        stmts.push_back(it->get());
        stmtsidMap[it->get()] = stmtsidMap.size();
    }

    bool stop, ugate;
    auto graph = BuildLayerGraph(stmts, qton);
    auto ref = graph;
    auto marked = std::vector<bool>(qton.getSize(), false);

    auto order = std::vector<unsigned>();

    do {
        Layer layer;
        stop = true;

        // Emit U-gates that may be executed in parallel.
        // However, we want to schedule the controlled gates only, as they
        // are the only gates that affects qubit allocation.
        do {
            ugate = false;

            for (unsigned i = 0, e = qton.getSize(); i < e; ++i) {
                if (ref[i]->qargsid.size() == 1) {
                    order.push_back(stmtsidMap[ref[i]->node]);

                    ref[i] = ref[i]->child[i];
                    ugate = true;
                }
            }
        } while (ugate);

        // Reach gates with non-marked qubits and mark them.
        for (unsigned i = 0, e = qton.getSize(); i < e; ++i) {
            if (!marked[i]) {
                marked[i] = true;
                --ref[i]->reached;
            }
        }

        // Advance the qubits' ref and unmark them.
        for (unsigned i = 0, e = qton.getSize(); i < e; ++i) {
            if (!ref[i]->reached) {
                order.push_back(stmtsidMap[ref[i]->node]);
                layer.insert(ref[i]->node);

                marked[i] = false;
                ref[i] = ref[i]->child[i];
            }

            // If ref isn't nullptr, it means that there still are
            // operations to emit.
            if (ref[i] != nullptr)
                stop = false;
        }

        mData.push_back(layer);
    } while (!stop);
}

efd::LayersBuilderPass::uRef efd::LayersBuilderPass::Create() {
    return uRef(new LayersBuilderPass());
}
