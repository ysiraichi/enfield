#include "enfield/Transform/RenameQbitsPass.h"

#include <cassert>

namespace efd {
    extern NDId::uRef SWAP_ID_NODE;
}

efd::RenameQbitPass::RenameQbitPass(ArchMap map) : mAMap(map) {
    mUK += Pass::K_STMT_PASS;
}

efd::Node::uRef efd::RenameQbitPass::getNodeFromOld(Node::Ref old) {
    std::string id = old->toString();
    assert(mAMap.find(id) != mAMap.end() && "Node not found for id/idref.");
    return mAMap[id]->clone();
}

bool efd::RenameQbitPass::isSwapGate(NDQOpGeneric::Ref ref) {
    return ref->getId()->toString() == SWAP_ID_NODE->toString();
}

void efd::RenameQbitPass::visit(NDQOpMeasure::Ref ref) {
    ref->setQBit(getNodeFromOld(ref->getQBit()));
}

void efd::RenameQbitPass::visit(NDQOpReset::Ref ref) {
    ref->setQArg(getNodeFromOld(ref->getQArg()));
}

void efd::RenameQbitPass::visit(NDQOpU::Ref ref) {
    ref->setQArg(getNodeFromOld(ref->getQArg()));
}

void efd::RenameQbitPass::visit(NDQOpCX::Ref ref) {
    ref->setLhs(getNodeFromOld(ref->getLhs()));
    ref->setRhs(getNodeFromOld(ref->getRhs()));
}

void efd::RenameQbitPass::visit(NDQOpBarrier::Ref ref) {
    ref->getQArgs()->apply(this);
}

void efd::RenameQbitPass::visit(NDQOpGeneric::Ref ref) {
    std::string lhs, rhs;

    // Will only swap the values in the map if this is a swap call, and
    // this call was created by the compiler.
    bool applySwapBetweenQbits = ref->wasGenerated() && isSwapGate(ref);
    if (applySwapBetweenQbits) {
        auto qArgs = ref->getQArgs();
        assert(qArgs->getChildNumber() == 2 && "Swap with more than two childrem.");
        // Must get the references before renaming.
        lhs = qArgs->getChild(0)->toString();
        rhs = qArgs->getChild(1)->toString();
    }

    // Rename the quantum arguments before swapping in order to preserve
    // the order.
    ref->getQArgs()->apply(this);

    if (applySwapBetweenQbits)
        std::swap(mAMap[lhs], mAMap[rhs]);
}

void efd::RenameQbitPass::visit(NDList::Ref ref) {
    for (unsigned i = 0, e = ref->getChildNumber(); i < e; ++i) {
        ref->setChild(i, getNodeFromOld(ref->getChild(i)));
    }
}

bool efd::RenameQbitPass::doesInvalidatesModule() const {
    return true;
}

efd::RenameQbitPass::uRef efd::RenameQbitPass::Create(ArchMap map) {
    return uRef(new RenameQbitPass(map));
}
