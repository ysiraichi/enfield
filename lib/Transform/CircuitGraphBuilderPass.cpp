#include "enfield/Transform/CircuitGraphBuilderPass.h"
#include "enfield/Transform/XbitToNumberPass.h"
#include "enfield/Transform/PassCache.h"
#include "enfield/Support/RTTI.h"

using namespace efd;

static void processCBit(uint32_t, CircuitNode*, CircuitGraph&, CircuitGraph&);
static void processQuBit(uint32_t, CircuitNode*, CircuitGraph&, CircuitGraph&);

uint8_t CircuitGraphBuilderPass::ID = 0;

bool CircuitGraphBuilderPass::run(QModule* qmod) {
    auto& graph = mData;
    CircuitGraph last;

    auto xtonpass = PassCache::Get<XbitToNumberWrapperPass>(qmod);
    auto xton = xtonpass->getData();

    auto qubits = xton.getQSize();
    auto cbits = xton.getCSize();
    graph.assign(qubits + cbits, nullptr);
    last.assign(qubits + cbits, nullptr);

    std::vector<Node::Ref> stmts;
    for (auto it = qmod->stmt_begin(), e = qmod->stmt_end(); it != e; ++it)
        stmts.push_back(it->get());

    for (auto node : stmts) {
        auto qopnode = dynCast<NDQOp>(node);
        auto parent = node;

        auto newnode = new CircuitNode();
        newnode->node = parent;

        // Processing the cbits, if necessary.
        if (auto ifstmt = dynCast<NDIfStmt>(node)) {
            qopnode = ifstmt->getQOp();
            auto cbitstr = ifstmt->getCondId()->getVal();

            for (auto cbit : xton.getRegUIds(cbitstr)) {
                processCBit(qubits + cbit, newnode, last, graph);
            }

        } else if (auto measure = dynCast<NDQOpMeasure>(node)) {
            auto cbitstr = measure->getCBit()->toString();
            processCBit(qubits + xton.getCUId(cbitstr), newnode, last, graph);
        }

        // Process the qubits.
        auto qargs = qopnode->getQArgs();

        for (uint32_t i = 0, e = qargs->getChildNumber(); i < e; ++i) {
            auto qarg = qargs->getChild(i);
            processQuBit(xton.getQUId(qarg->toString()), newnode, last, graph);
        }
    }

    return false;
}

CircuitGraphBuilderPass::uRef CircuitGraphBuilderPass::Create() {
    return uRef(new CircuitGraphBuilderPass());
}

// ----------------------------------------------------------------
// --------------------- Static Functions -------------------------
// ----------------------------------------------------------------

static void processCBit(uint32_t cbitid, CircuitNode* newnode,
                        CircuitGraph& last, CircuitGraph& graph) {
    newnode->cargsid.insert(cbitid);
    
    if (last[cbitid] == nullptr) {
        graph[cbitid] = newnode;
    } else {
        last[cbitid]->child[cbitid] = newnode;
    }
    
    newnode->child[cbitid] = nullptr;
    last[cbitid] = newnode;
}

static void processQuBit(uint32_t qubitid, CircuitNode* newnode,
                        CircuitGraph& last, CircuitGraph& graph) {
    newnode->qargsid.insert(qubitid);
    
    if (last[qubitid] == nullptr) {
        graph[qubitid] = newnode;
    } else {
        last[qubitid]->child[qubitid] = newnode;
    }
    
    newnode->child[qubitid] = nullptr;
    last[qubitid] = newnode;
}
