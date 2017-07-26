#include "enfield/Transform/ReverseEdgesPass.h"
#include "enfield/Transform/Utils.h"
#include "enfield/Analysis/NodeVisitor.h"
#include "enfield/Support/RTTI.h"

#include <cassert>

namespace efd {
    class ReverseEdgesVisitor : public NodeVisitor {
        private:
            ArchGraph::sRef mG;

        public:
            std::vector<Node::Ref> mRevVector;
            ReverseEdgesVisitor(ArchGraph::sRef g) : mG(g) {}

            void visit(NDQOpCX::Ref ref) override;
            void visit(NDQOpGeneric::Ref ref) override;
    };
}

void efd::ReverseEdgesVisitor::visit(NDQOpCX::Ref ref) {
    unsigned uidLhs = mG->getUId(ref->getLhs()->toString());
    unsigned uidRhs = mG->getUId(ref->getRhs()->toString());

    if (mG->isReverseEdge(uidLhs, uidRhs)) {
        mRevVector.push_back(ref);
    }
}

void efd::ReverseEdgesVisitor::visit(NDQOpGeneric::Ref ref) {
    if (ref->getId()->getVal() == "cx") {
        // Have to come up a way to overcome this.
        assert(ref->getQArgs()->getChildNumber() == 2 && "CNot gate malformed.");
        NDList* qargs = ref->getQArgs();
        unsigned uidLhs = mG->getUId(qargs->getChild(0)->toString());
        unsigned uidRhs = mG->getUId(qargs->getChild(1)->toString());

        if (mG->isReverseEdge(uidLhs, uidRhs)) {
            mRevVector.push_back(ref);
        }
    }
}

efd::ReverseEdgesPass::ReverseEdgesPass(ArchGraph::sRef graph) : mG(graph) {
}

void efd::ReverseEdgesPass::run(QModule::Ref qmod) {
    ReverseEdgesVisitor visitor(mG);

    for (auto it = qmod->stmt_begin(), e = qmod->stmt_end(); it != e; ++it) {
        (*it)->apply(&visitor);
    }

    for (auto node : visitor.mRevVector) {
        ReverseCNode(qmod, node);
    }
}

efd::ReverseEdgesPass::uRef efd::ReverseEdgesPass::Create(ArchGraph::sRef graph) {
    return uRef(new ReverseEdgesPass(graph));
}
