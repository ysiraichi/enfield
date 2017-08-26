#include "enfield/Transform/InlineAllPass.h"
#include "enfield/Analysis/NodeVisitor.h"

namespace efd {
    class InlineAllVisitor : public NodeVisitor {
        private:
            QModule& mMod;
            std::set<std::string> mBasis;

        public:
            std::vector<NDQOp::Ref> mInlineVector;

            InlineAllVisitor(QModule& qmod, std::set<std::string> basis)
                : mMod(qmod), mBasis(basis) {}

            void visit(NDQOp::Ref ref) override;
            void visit(NDIfStmt::Ref ref) override;
    };
}

void efd::InlineAllVisitor::visit(NDQOp::Ref ref) {
    if (mBasis.find(ref->getId()->getVal()) == mBasis.end()) {
        mInlineVector.push_back(ref);
    }
}

void efd::InlineAllVisitor::visit(NDIfStmt::Ref ref) {
    ref->getQOp()->apply(this);
}

efd::InlineAllPass::InlineAllPass(std::vector<std::string> basis) {
    mBasis = std::set<std::string>(basis.begin(), basis.end());
}

void efd::InlineAllPass::run(QModule::Ref qmod) {
    InlineAllVisitor visitor(*qmod, mBasis);

    do {
        visitor.mInlineVector.clear();
        // Inline until we can't inline anything anymore.
        for (auto it = qmod->stmt_begin(), e = qmod->stmt_end(); it != e; ++it) {
            (*it)->apply(&visitor);
        }

        for (auto call : visitor.mInlineVector) {
            auto sign = qmod->getQGate(call->getId()->getVal());

            // Inline only non-opaque gates.
            if (!sign->isOpaque()) {
                qmod->inlineCall(call);
            }
        }
    } while (!visitor.mInlineVector.empty());
}

efd::InlineAllPass::uRef efd::InlineAllPass::Create(std::vector<std::string> basis) {
    return uRef(new InlineAllPass(basis));
}
