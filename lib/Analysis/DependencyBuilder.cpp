#include "enfield/Analysis/DependencyBuilder.h"

void efd::DependencyBuilderPass::visit(NDGateDecl* ref) {
    mCurrentGate = ref;
    ref->getGOpList()->apply(this);
}

void efd::DependencyBuilderPass::visit(NDGOpList* ref) {
    for (auto childRef : *ref)
        childRef->apply(this);
}

void efd::DependencyBuilderPass::visit(NDQOpCX* ref) {
}
