#include "enfield/Analysis/QModulefyPass.h"
#include "enfield/Analysis/IdTable.h"
#include "enfield/Support/RTTI.h"

efd::QModulefyPass::QModulefyPass() {
    mUK = Pass::K_AST_PASS;
}

void efd::QModulefyPass::initImpl() {
    mMod = new QModule();
    mCurrentTable = mMod->mTable;
}

void efd::QModulefyPass::visit(NDQasmVersion* ref) {
    if (mMod->mVersion == nullptr)
        mMod->mVersion = ref;
    ref->getStatements()->apply(this);
}

void efd::QModulefyPass::visit(NDInclude* ref) {
    ref->getStatements()->apply(this);
}

void efd::QModulefyPass::visit(NDDecl* ref) {
    mMod->mRegDecls.push_back(ref);
    mCurrentTable->addQVar(ref->getId()->getVal(), ref);
}

void efd::QModulefyPass::visit(NDGateDecl* ref) {
    mMod->mGates.push_back(ref);
    mCurrentTable->addQGate(ref->getId()->getVal(), ref);

    mCurrentTable = IdTable::create(mCurrentTable);
    mMod->mIdTableMap[ref] = mCurrentTable;
    for (auto varRef : *ref->getQArgs()) {
        std::string id = dynCast<NDId>(varRef)->getVal();
        mCurrentTable->addQVar(id, varRef);
    }
    mCurrentTable = mCurrentTable->getParent();
}

void efd::QModulefyPass::visit(NDOpaque* ref) {
    mMod->mGates.push_back(ref);

    mCurrentTable->addQGate(ref->getId()->getVal(), ref);
    mMod->mIdTableMap[ref] = mCurrentTable;
    mCurrentTable = IdTable::create(mCurrentTable);
    for (auto varRef : *ref->getQArgs()) {
        std::string id = dynCast<NDId>(varRef)->getVal();
        mCurrentTable->addQVar(id, varRef);
    }
    mCurrentTable = mCurrentTable->getParent();
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

