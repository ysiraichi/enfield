#include "enfield/Analysis/QModulefyPass.h"

efd::QModulefyPass::QModulefyPass() {
    mMod = new QModule();
}

void efd::QModulefyPass::visit(NDQasmVersion* ref) {
    mMod->mVersion = ref;
    ref->getStatements()->apply(this);
}

void efd::QModulefyPass::visit(NDInclude* ref) {
    ref->getStatements()->apply(this);
}

void efd::QModulefyPass::visit(NDDecl* ref) {
    mMod->mRegDecls.push_back(ref);
}

void efd::QModulefyPass::visit(NDGateDecl* ref) {
    mMod->mGates.push_back(ref);
}

void efd::QModulefyPass::visit(NDOpaque* ref) {
    mMod->mGates.push_back(ref);
}

void efd::QModulefyPass::visit(NDQOpMeasure* ref) {
    mMod->mStatements.push_back(ref);
}

void efd::QModulefyPass::visit(NDQOpReset* ref) {
    mMod->mStatements.push_back(ref);
}

void efd::QModulefyPass::visit(NDQOpU* ref) {
    mMod->mStatements.push_back(ref);
}

void efd::QModulefyPass::visit(NDQOpCX* ref) {
    mMod->mStatements.push_back(ref);
}

void efd::QModulefyPass::visit(NDQOpBarrier* ref) {
    mMod->mStatements.push_back(ref);
}

void efd::QModulefyPass::visit(NDQOpGeneric* ref) {
    mMod->mStatements.push_back(ref);
}

void efd::QModulefyPass::visit(NDStmtList* ref) {
    for (auto child : *ref)
        child->apply(this);
}

void efd::QModulefyPass::visit(NDIfStmt* ref) {
    mMod->mStatements.push_back(ref);
}

efd::QModulefyPass* efd::QModulefyPass::Create() {
    return new QModulefyPass();
}

