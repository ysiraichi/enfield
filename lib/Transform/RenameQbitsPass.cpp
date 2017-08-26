#include "enfield/Transform/RenameQbitsPass.h"
#include "enfield/Analysis/NodeVisitor.h"

#include <cassert>

namespace efd {
    class RenameQbitVisitor : public NodeVisitor {
        private:
            RenameQbitPass::ArchMap& mAMap;

            /// \brief Gets the node associated with the old node (that is currently
            /// inside)
            Node::uRef getNodeFromOld(Node::Ref old);

        public:
            RenameQbitVisitor(RenameQbitPass::ArchMap& map) : mAMap(map) {}

            void visit(NDQOpMeasure::Ref ref) override;
            void visit(NDQOpReset::Ref ref) override;
            void visit(NDQOpU::Ref ref) override;
            void visit(NDQOpCX::Ref ref) override;
            void visit(NDQOpBarrier::Ref ref) override;
            void visit(NDQOp::Ref ref) override;
            void visit(NDList::Ref ref) override;
    };
}

efd::Node::uRef efd::RenameQbitVisitor::getNodeFromOld(Node::Ref old) {
    std::string id = old->toString();
    assert(mAMap.find(id) != mAMap.end() && "Node not found for id/idref.");
    return mAMap[id]->clone();
}

void efd::RenameQbitVisitor::visit(NDQOpMeasure::Ref ref) {
    ref->setQBit(getNodeFromOld(ref->getQBit()));
}

void efd::RenameQbitVisitor::visit(NDQOpReset::Ref ref) {
    ref->setQArg(getNodeFromOld(ref->getQArg()));
}

void efd::RenameQbitVisitor::visit(NDQOpU::Ref ref) {
    ref->setQArg(getNodeFromOld(ref->getQArg()));
}

void efd::RenameQbitVisitor::visit(NDQOpCX::Ref ref) {
    ref->setLhs(getNodeFromOld(ref->getLhs()));
    ref->setRhs(getNodeFromOld(ref->getRhs()));
}

void efd::RenameQbitVisitor::visit(NDQOpBarrier::Ref ref) {
    ref->getQArgs()->apply(this);
}

void efd::RenameQbitVisitor::visit(NDQOp::Ref ref) {
    // Rename the quantum arguments before swapping in order to preserve
    // the order.
    ref->getQArgs()->apply(this);
}

void efd::RenameQbitVisitor::visit(NDList::Ref ref) {
    for (unsigned i = 0, e = ref->getChildNumber(); i < e; ++i) {
        ref->setChild(i, getNodeFromOld(ref->getChild(i)));
    }
}

efd::RenameQbitPass::RenameQbitPass(ArchMap map) : mAMap(map) {
}

void efd::RenameQbitPass::run(QModule::Ref qmod) {
    RenameQbitVisitor visitor(mAMap);

    for (auto it = qmod->stmt_begin(), e = qmod->stmt_end(); it != e; ++it) {
        (*it)->apply(&visitor);
    }
}

efd::RenameQbitPass::uRef efd::RenameQbitPass::Create(ArchMap map) {
    return uRef(new RenameQbitPass(map));
}
