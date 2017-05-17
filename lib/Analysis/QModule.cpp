#include "enfield/Analysis/Driver.h"
#include "enfield/Analysis/QModule.h"
#include "enfield/Analysis/QModulefyPass.h"
#include "enfield/Analysis/IdTable.h"
#include "enfield/Analysis/Pass.h"

efd::QModule::QModule() : mAST(nullptr), mVersion(nullptr) {
    mTable = IdTable::create();
}

efd::QModule::Iterator efd::QModule::reg_begin() {
    return mRegDecls.begin();
}

efd::QModule::ConstIterator efd::QModule::reg_begin() const {
    return mRegDecls.begin();
}

efd::QModule::Iterator efd::QModule::reg_end() {
    return mRegDecls.end();
}

efd::QModule::ConstIterator efd::QModule::reg_end() const {
    return mRegDecls.end();
}

efd::QModule::Iterator efd::QModule::gates_begin() {
    return mGates.begin();
}

efd::QModule::ConstIterator efd::QModule::gates_begin() const {
    return mGates.begin();
}

efd::QModule::Iterator efd::QModule::gates_end() {
    return mGates.end();
}

efd::QModule::ConstIterator efd::QModule::gates_end() const {
    return mGates.end();
}

efd::QModule::Iterator efd::QModule::stmt_begin() {
    return mStatements.begin();
}

efd::QModule::ConstIterator efd::QModule::stmt_begin() const {
    return mStatements.begin();
}

efd::QModule::Iterator efd::QModule::stmt_end() {
    return mStatements.end();
}

efd::QModule::ConstIterator efd::QModule::stmt_end() const {
    return mStatements.end();
}

void efd::QModule::print(std::ostream& O, bool pretty) const {
    O << toString(pretty);
}

std::string efd::QModule::toString(bool pretty) const {
    return mAST->toString(pretty);
}

efd::IdTable* efd::QModule::getIdTable(NodeRef ref) {
    if (mIdTableMap.find(ref) != mIdTableMap.end())
        return mIdTableMap[ref];
    return nullptr;
}

efd::NodeRef efd::QModule::getQVar(std::string id, bool recursive) {
    return mTable->getQVar(id, recursive);
}

efd::NodeRef efd::QModule::getQGate(std::string id, bool recursive) {
    return mTable->getQGate(id, recursive);
}

void efd::QModule::runPass(Pass* pass, bool force) {
    if (pass->wasApplied() && !force)
        return;

    pass->init();

    if (pass->isASTPass()) {
        mAST->apply(pass);
    } else if (pass->isRegDeclPass()) {
        for (auto regdecl : mRegDecls)
            regdecl->apply(pass);
    } else if (pass->isGatePass()) {
        for (auto gate : mGates)
            gate->apply(pass);
    } else {
        // if (pass->isStatementPass())
        for (auto stmt : mStatements)
            stmt->apply(pass);
    }
}

std::unique_ptr<efd::QModule> efd::QModule::GetFromAST(NodeRef ref) {
    QModulefyPass* pass = QModulefyPass::Create();
    ref->apply(pass);
    std::unique_ptr<QModule> qmod(pass->mMod);
    qmod->mAST = ref;
    return qmod;
}

std::unique_ptr<efd::QModule> efd::QModule::Parse(std::string filename, std::string path) {
    NodeRef ast = efd::ParseFile(filename, path);

    if (ast != nullptr)
        return GetFromAST(ast);

    return std::unique_ptr<QModule>(nullptr);
}

std::unique_ptr<efd::QModule> efd::QModule::ParseString(std::string program) {
    NodeRef ast = efd::ParseString(program);

    if (ast != nullptr)
        return GetFromAST(ast);

    return std::unique_ptr<QModule>(nullptr);
}
