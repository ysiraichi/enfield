#include "enfield/Transform/ReverseEdgesPass.h"
#include "enfield/Transform/Utils.h"
#include "enfield/Support/RTTI.h"

#include <cassert>

efd::ReverseEdgesPass::ReverseEdgesPass(QModule* qmod, Graph* graph) :
    mMod(qmod), mG(graph) {
    mDepPass = DependencyBuilderPass::Create(qmod);
    mUK += Pass::K_STMT_PASS;
}

void efd::ReverseEdgesPass::initImpl(bool force) {
    mMod->runPass(mDepPass, force);
}

void efd::ReverseEdgesPass::visit(NDQOpCX* ref) {
    unsigned uidLhs = mG->getUId(ref->getLhs()->toString());
    unsigned uidRhs = mG->getUId(ref->getRhs()->toString());

    if (mG->isReverseEdge(uidLhs, uidRhs)) {
        ReverseCNode(ref);
    }
}

void efd::ReverseEdgesPass::visit(NDQOpGeneric* ref) {
    if (ref->getId()->getVal() == "cx") {
        // Have to come up a way to overcome this.
        assert(ref->getQArgs()->getChildNumber() == 2 && "CNot gate malformed.");
        NDList* qargs = ref->getQArgs();
        unsigned uidLhs = mG->getUId(qargs->getChild(0)->toString());
        unsigned uidRhs = mG->getUId(qargs->getChild(1)->toString());

        if (mG->isReverseEdge(uidLhs, uidRhs)) {
            ReverseCNode(ref);
        }
    }
}

bool efd::ReverseEdgesPass::doesInvalidatesModule() const {
    return true;
}

efd::ReverseEdgesPass* efd::ReverseEdgesPass::Create(QModule* qmod, Graph* graph) {
    return new ReverseEdgesPass(qmod, graph);
}
