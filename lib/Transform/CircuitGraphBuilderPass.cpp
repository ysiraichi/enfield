#include "enfield/Transform/CircuitGraphBuilderPass.h"
#include "enfield/Transform/XbitToNumberPass.h"
#include "enfield/Transform/PassCache.h"
#include "enfield/Support/RTTI.h"

unsigned efd::CircuitGraphBuilderPass::ID = 0;

bool efd::CircuitGraphBuilderPass::run(QModule* qmod) {
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

        if (auto ifstmt = dynCast<NDIfStmt>(node)) {
            qopnode = ifstmt->getQOp();

            auto cbitstr = ifstmt->getCondId()->getVal();
            for (auto cbit : xton.getRegUIds(cbitstr)) {
                auto cbitid = qubits + cbit;

                newnode->cargsid.insert(cbitid);

                if (last[cbitid] == nullptr) {
                    graph[cbitid] = newnode;
                } else {
                    last[cbitid]->child[cbitid] = newnode;
                }

                last[cbitid] = newnode;
            }
        } else if (auto measure = dynCast<NDQOpMeasure>(node)) {
            auto cbitstr = measure->getCBit()->toString();
            auto cbitid = qubits + xton.getCUId(cbitstr);

            newnode->cargsid.insert(cbitid);

            if (last[cbitid] == nullptr) {
                graph[cbitid] = newnode;
            } else {
                last[cbitid]->child[cbitid] = newnode;
            }

            last[cbitid] = newnode;
        }

        auto qargs = qopnode->getQArgs();
        for (unsigned i = 0, e = qargs->getChildNumber(); i < e; ++i) {
            auto qarg = qargs->getChild(i);
            auto qargid = xton.getQUId(qarg->toString());

            newnode->qargsid.insert(qargid);

            if (last[qargid] == nullptr) {
                graph[qargid] = newnode;
            } else {
                last[qargid]->child[qargid] = newnode;
            }

            last[qargid] = newnode;
        }
    }

    return false;
}

efd::CircuitGraphBuilderPass::uRef efd::CircuitGraphBuilderPass::Create() {
    return uRef(new CircuitGraphBuilderPass());
}
