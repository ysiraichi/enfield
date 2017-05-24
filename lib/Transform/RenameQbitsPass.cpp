#include "enfield/Transform/RenameQbitsPass.h"

#include <cassert>

namespace efd {
    extern const NDId* SWAP_ID_NODE;
}

efd::RenameQbitPass::RenameQbitPass(ArchMap map) : mAMap(map) {
}

efd::NodeRef efd::RenameQbitPass::getNodeFromOld(NodeRef old) {
    std::string id = old->toString();
    assert(mAMap.find(id) != mAMap.end() && "Node not found for id/idref.");
    return mAMap[id]->clone();
}

bool efd::RenameQbitPass::isSwapGate(NDQOpGeneric* ref) {
    return ref->getId()->toString() == SWAP_ID_NODE->toString();
}

void efd::RenameQbitPass::visit(NDQOpMeasure* ref) {
    ref->setQBit(getNodeFromOld(ref->getQBit()));
}

void efd::RenameQbitPass::visit(NDQOpReset* ref) {
    ref->setQArg(getNodeFromOld(ref->getQArg()));
}

void efd::RenameQbitPass::visit(NDQOpU* ref) {
    ref->setQArg(getNodeFromOld(ref->getQArg()));
}

void efd::RenameQbitPass::visit(NDQOpCX* ref) {
    ref->setLhs(getNodeFromOld(ref->getLhs()));
    ref->setRhs(getNodeFromOld(ref->getRhs()));
}

void efd::RenameQbitPass::visit(NDQOpBarrier* ref) {
    ref->getQArgs()->apply(this);
}

void efd::RenameQbitPass::visit(NDQOpGeneric* ref) {
    ref->getQArgs()->apply(this);
    if (ref->wasGenerated() && isSwapGate(ref)) {
        auto qArgs = ref->getQArgs();
        auto childLhs = qArgs->getChild(0);
        auto childRhs = qArgs->getChild(1);
        assert(mAMap.find(childLhs->toString()) != mAMap.end() &&
                "Swap LHS node mapping not found.");
        assert(mAMap.find(childRhs->toString()) != mAMap.end() &&
                "Swap RHS node mapping not found.");
        std::swap(mAMap[childLhs->toString()], mAMap[childRhs->toString()]);
    }
}

void efd::RenameQbitPass::visit(NDList* ref) {
    for (unsigned i = 0, e = ref->getChildNumber(); i < e; ++i) {
        ref->setChild(i, getNodeFromOld(ref->getChild(i)));
    }
}

efd::RenameQbitPass* efd::RenameQbitPass::Create(ArchMap map) {
    return new RenameQbitPass(map);
}
