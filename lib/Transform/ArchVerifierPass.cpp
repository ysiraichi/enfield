#include "enfield/Transform/ArchVerifierPass.h"
#include "enfield/Transform/Utils.h"
#include "enfield/Analysis/NodeVisitor.h"


using namespace efd;

namespace efd {
    class ArchVerifierVisitor : public NodeVisitor {
        private:
            struct SemanticCNOT { uint32_t u, v; };

            ArchGraph::sRef mArch;

            bool checkCNOT(SemanticCNOT cnot);
            bool visitNDQOp(NDQOp::Ref qop);

        public:
            ArchVerifierVisitor(ArchGraph::sRef ag);

            void visit(NDQOpMeasure::Ref ref) override;
            void visit(NDQOpReset::Ref ref) override;
            void visit(NDQOpU::Ref ref) override;
            void visit(NDQOpBarrier::Ref ref) override;
            void visit(NDQOpCX::Ref ref) override;
            void visit(NDQOpGen::Ref ref) override;
            void visit(NDIfStmt::Ref ref) override;

            bool mSuccess;
    };
}

ArchVerifierVisitor::ArchVerifierVisitor(ArchGraph::sRef ag) : mArch(ag), mSuccess(true) {}

bool ArchVerifierVisitor::checkCNOT(SemanticCNOT cnot) {
    return mArch->hasEdge(cnot.u, cnot.v);
}

bool ArchVerifierVisitor::visitNDQOp(NDQOp::Ref qop) {
    bool success;

    std::vector<uint32_t> qUIds;
    for (auto& qarg : *qop->getQArgs()) {
        auto qargStr = qarg->toString(false);

        if (mArch->hasSId(qargStr)) {
            qUIds.push_back(mArch->getUId(qargStr));
        } else {
            // If there is some quantum operation that uses an inexistent qubit, we already
            // may return false!
            return false;
        }
    }

    if (IsCNOTGateCall(qop)) {
        EfdAbortIf(qUIds.size() != 2,
                   "CNOT call must have 2 quantum qubits. Actual: `"
                   << qop->toString(false) << "`.");

        success = checkCNOT({ qUIds[0], qUIds[1] });

    } else if (IsIntrinsicGateCall(qop)) {
        auto gen = dynCast<NDQOpGen>(qop);

        EfdAbortIf(gen == nullptr,
                   "Intrinsic call not generic operation!? Actual: `"
                   << qop->toString(false) << "`.");

        EfdAbortIf(!gen->isIntrinsic(),
                   "Intrinsic call not marked as intrinsic!? Actual: `"
                   << qop->toString(false) << "`.");

        switch (gen->getIntrinsicKind()) {
            case NDQOpGen::K_INTRINSIC_SWAP:
                EfdAbortIf(qUIds.size() != 2,
                           "SWAP call must have 2 quantum qubits. Actual: `"
                           << qUIds.size() << "`.");

                success = checkCNOT({ qUIds[0], qUIds[1] }) || checkCNOT({ qUIds[1], qUIds[0] });
                break;

            case NDQOpGen::K_INTRINSIC_LCX:
                EfdAbortIf(qUIds.size() != 3,
                           "LCX call must have 3 quantum qubits. Actual: `"
                           << qUIds.size() << "`.");

                success = (checkCNOT({ qUIds[0], qUIds[1] }) || checkCNOT({ qUIds[1], qUIds[0] })) &&
                          (checkCNOT({ qUIds[1], qUIds[2] }) || checkCNOT({ qUIds[2], qUIds[1] }));
                break;

            case NDQOpGen::K_INTRINSIC_REV_CX:
                EfdAbortIf(qUIds.size() != 2,
                           "Reversal call must have 2 quantum qubits. Actual: `"
                           << qUIds.size() << "`.");

                success = checkCNOT({ qUIds[1], qUIds[0] });
                break;
        }
    } else {
        success = true;
    }

    return success;
}

void ArchVerifierVisitor::visit(NDQOpMeasure::Ref ref) {
    mSuccess = mSuccess && visitNDQOp(ref);
}

void ArchVerifierVisitor::visit(NDQOpReset::Ref ref) {
    mSuccess = mSuccess && visitNDQOp(ref);
}

void ArchVerifierVisitor::visit(NDQOpU::Ref ref) {
    mSuccess = mSuccess && visitNDQOp(ref);
}

void ArchVerifierVisitor::visit(NDQOpBarrier::Ref ref) {
    mSuccess = mSuccess && visitNDQOp(ref);
}

void ArchVerifierVisitor::visit(NDQOpCX::Ref ref) {
    mSuccess = mSuccess && visitNDQOp(ref);
}

void ArchVerifierVisitor::visit(NDQOpGen::Ref ref) {
    mSuccess = mSuccess && visitNDQOp(ref);
}

void ArchVerifierVisitor::visit(NDIfStmt::Ref ref) {
    mSuccess = mSuccess && visitNDQOp(ref->getQOp());
}

// ----------------------------------------------------------------
// --------------------- ArchVerifierPass -------------------------
// ----------------------------------------------------------------

ArchVerifierPass::ArchVerifierPass(ArchGraph::sRef ag) : mArch(ag) {
    mData = false;
}

bool ArchVerifierPass::run(QModule* qmod) {
    mData = true;
    ArchVerifierVisitor visitor(mArch);

    for (auto it = qmod->stmt_begin(), end = qmod->stmt_end();
            it != end && mData; ++it) {
        (*it)->apply(&visitor);
        mData = mData && visitor.mSuccess;
    }

    return false;
}

ArchVerifierPass::uRef ArchVerifierPass::Create(ArchGraph::sRef ag) {
    return uRef(new ArchVerifierPass(ag));
}
