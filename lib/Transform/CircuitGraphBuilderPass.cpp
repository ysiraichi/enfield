#include "enfield/Transform/CircuitGraphBuilderPass.h"
#include "enfield/Transform/DependencyBuilderPass.h"
#include "enfield/Support/RTTI.h"

void efd::CircuitGraphBuilderPass::run(QModule* qmod) {
    auto& graph = mData;
    CircuitGraph last;

    auto qtonpass = QbitToNumberWrapperPass::Create();
    qtonpass->run(qmod);
    auto qton = qtonpass->getData();

    graph.assign(qton.getSize(), nullptr);
    last.assign(qton.getSize(), nullptr);

    std::vector<Node::Ref> stmts;
    for (auto it = qmod->stmt_begin(), e = qmod->stmt_end(); it != e; ++it)
        stmts.push_back(it->get());

    for (auto node : stmts) {
        auto qopnode = dynCast<NDQOp>(node);
        auto parent = node;

        if (auto ifstmt = dynCast<NDIfStmt>(node)) {
            qopnode = ifstmt->getQOp();
        }

        auto qargs = qopnode->getQArgs();
        auto newnode = new CircuitNode();

        newnode->node = parent;
        newnode->child = std::vector<CircuitNode*>(qton.getSize(), nullptr);

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
}

efd::CircuitGraphBuilderPass::uRef efd::CircuitGraphBuilderPass::Create() {
    return uRef(new CircuitGraphBuilderPass());
}
