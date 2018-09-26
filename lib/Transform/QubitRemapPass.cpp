#include "enfield/Transform/QubitRemapPass.h"
#include "enfield/Transform/PassCache.h"

using namespace efd;

// ==--------------- QubitRemapVisitor ---------------==
QubitRemapVisitor::QubitRemapVisitor(const Mapping& m, const XbitToNumber& xtoN)
    : mMap(m), mXtoN(xtoN) {}

bool QubitRemapVisitor::wasReplaced() {
    return mWasReplaced;
}

void QubitRemapVisitor::visitNDQOp(NDQOp::Ref qop) {
    mWasReplaced = true;
    auto qargs = qop->getQArgs();
    NDList::uRef newQArgs = NDList::Create();

    for (auto& qarg : *qargs) {
        uint32_t pseudoQUId = mXtoN.getQUId(qarg->toString(false));
        uint32_t physicalQUId = mMap[pseudoQUId];

        if (physicalQUId != _undef) {
            newQArgs->addChild(mXtoN.getQNode(physicalQUId)->clone());
        } else {
            newQArgs->addChild(qarg->clone());
            mWasReplaced = false;
            break;
        }
    }

    if (mWasReplaced) {
        qop->setQArgs(std::move(newQArgs));
    }
}

void QubitRemapVisitor::visit(NDQOpMeasure::Ref ref) {
    visitNDQOp((NDQOp::Ref) ref);
}

void QubitRemapVisitor::visit(NDQOpReset::Ref ref) {
    visitNDQOp((NDQOp::Ref) ref);
}

void QubitRemapVisitor::visit(NDQOpU::Ref ref) {
    visitNDQOp((NDQOp::Ref) ref);
}

void QubitRemapVisitor::visit(NDQOpCX::Ref ref) {
    visitNDQOp((NDQOp::Ref) ref);
}

void QubitRemapVisitor::visit(NDQOpBarrier::Ref ref) {
    visitNDQOp((NDQOp::Ref) ref);
}

void QubitRemapVisitor::visit(NDQOpGen::Ref ref) {
    visitNDQOp((NDQOp::Ref) ref);
}

void QubitRemapVisitor::visit(NDIfStmt::Ref ref) {
    ref->getQOp()->apply(this);
}

// ==--------------- QubitRemapPass ---------------==
QubitRemapPass::QubitRemapPass(const Mapping& m) : mMap(m) {}

bool QubitRemapPass::run(QModule* qmod) {
    auto xbitToN = PassCache::Get<XbitToNumberWrapperPass>(qmod)->getData();
    QubitRemapVisitor visitor(mMap, xbitToN);

    for (auto it = qmod->stmt_begin(), end = qmod->stmt_end(); it != end; ++it) {
        (*it)->apply(&visitor);
    }

    return true;
}

QubitRemapPass::uRef QubitRemapPass::Create(const Mapping& m) {
    return uRef(new QubitRemapPass(m));
}
