#include "enfield/Transform/QModulefyPass.h"
#include "enfield/Transform/IdTable.h"
#include "enfield/Support/RTTI.h"

#include <unordered_map>

extern std::unordered_map<std::string, std::string> StdLib;

efd::QModulefyPass::QModulefyPass(QModule* qmod) : mMod(qmod) {
    mUK = Pass::K_AST_PASS;
}

void efd::QModulefyPass::initImpl(bool force) {
    mCurrentTable = &mMod->mTable;
    mIncludes.clear();
}

void efd::QModulefyPass::visit(NDQasmVersion* ref) {
    if (mMod->mVersion == nullptr)
        mMod->mVersion = ref;
    ref->getStatements()->apply(this);
}

void efd::QModulefyPass::visit(NDInclude* ref) {
    mIncludes.insert(ref->getFilename()->getVal());
    if (mIncludes.size() == StdLib.size())
        mMod->mStdLibsParsed = true;
    ref->getStatements()->apply(this);
}

void efd::QModulefyPass::visit(NDDecl* ref) {
    mMod->mRegDecls.push_back(ref);
    mCurrentTable->addQVar(ref->getId()->getVal(), ref);
}

void efd::QModulefyPass::visit(NDGateDecl* ref) {
    mMod->mGates.push_back(ref);
    mCurrentTable->addQGate(ref->getId()->getVal(), ref);

    mMod->mIdTableMap[ref] = IdTable(mCurrentTable);
    mCurrentTable = &mMod->mIdTableMap[ref];
    for (auto varRef : *ref->getQArgs()) {
        std::string id = dynCast<NDId>(varRef)->getVal();
        mCurrentTable->addQVar(id, varRef);
    }
    mCurrentTable = mCurrentTable->getParent();
}

void efd::QModulefyPass::visit(NDOpaque* ref) {
    mMod->mGates.push_back(ref);
    mCurrentTable->addQGate(ref->getId()->getVal(), ref);

    mMod->mIdTableMap[ref] = IdTable(mCurrentTable);
    mCurrentTable = &mMod->mIdTableMap[ref];
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

efd::QModulefyPass* efd::QModulefyPass::Create(QModule* qmod) {
    return new QModulefyPass(qmod);
}

