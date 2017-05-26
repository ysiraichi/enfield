#include "enfield/Analysis/Driver.h"
#include "enfield/Transform/QModule.h"
#include "enfield/Transform/QModulefyPass.h"
#include "enfield/Transform/IdTable.h"
#include "enfield/Transform/Utils.h"
#include "enfield/Support/RTTI.h"
#include "enfield/Pass.h"

#include <cassert>
#include <iterator>

efd::QModule::QModule(NodeRef ref) : mAST(ref), mVersion(nullptr), 
    mQModulefy(nullptr), mValid(true) {
}

efd::NodeRef efd::QModule::getVersion() {
    return mVersion;
}

efd::QModule::Iterator efd::QModule::insertGate(NDGateDecl* gate) {
    NDStmtList* stmts;
    Iterator it;

    if (auto version = dynCast<NDQasmVersion>(mAST))
        stmts = version->getStatements();
    else
        stmts = dynCast<NDStmtList>(mAST);

    if (stmts == nullptr) {
        assert(mAST == nullptr &&
                "AST root node is neither the version nor a statement list.");
        stmts = dynCast<NDStmtList>(NDStmtList::Create());
    }

    it = stmts->begin();
    stmts->addChild(it, gate);

    invalidate();
    return it;
}

efd::QModule::Iterator efd::QModule::inlineCall(NDQOpGeneric* call) {
    NodeRef parent = call->getParent();
    Iterator it = parent->findChild(call);
    unsigned dist = std::distance(parent->begin(), it);

    InlineGate(this, call);
    invalidate();
    return parent->begin() + dist;
}

efd::QModule::Iterator efd::QModule::insertNodeAfter(Iterator it, NodeRef ref) {
    InsertNodeAfter(it, ref);
    invalidate();
    return it;
}

efd::QModule::Iterator efd::QModule::insertNodeBefore(Iterator it, NodeRef ref) {
    InsertNodeBefore(it, ref);
    invalidate();
    return it;
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

efd::IdTable& efd::QModule::getIdTable(NDGateDecl* ref) {
    if (ref != nullptr && mIdTableMap.find(ref) != mIdTableMap.end())
        return mIdTableMap[ref];
    return mTable;
}

efd::NodeRef efd::QModule::getQVar(std::string id, NDGateDecl* gate, bool recursive) {
    IdTable& gTable = getIdTable(gate);
    return gTable.getQVar(id, recursive);
}

efd::NDGateDecl* efd::QModule::getQGate(std::string id, bool recursive) {
    return mTable.getQGate(id, recursive);
}

void efd::QModule::invalidate() {
    mVersion = nullptr;
    mRegDecls.clear();
    mGates.clear();
    mStatements.clear();
    mValid = false;
}

void efd::QModule::validate() {
    if (mQModulefy == nullptr)
        mQModulefy = QModulefyPass::Create(this);

    // mValid must be set before calling 'runPass'. Otherwise,
    // infinite loop!.
    mValid = true;
    runPass(mQModulefy, true);
}

bool efd::QModule::isValid() {
    return mValid;
}

void efd::QModule::runPass(Pass* pass, bool force) {
    if (pass->wasApplied() && !force)
        return;

    if (!isValid()) validate();

    pass->init();

    if (pass->isASTPass()) {
        mAST->apply(pass);
    } else {
        if (pass->isRegDeclPass()) {
            for (auto regdecl : mRegDecls)
                regdecl->apply(pass);
        }
        
        if (pass->isGatePass()) {
            for (auto gate : mGates)
                gate->apply(pass);
        }

        if (pass->isStatementPass()) {
            for (auto stmt : mStatements)
                stmt->apply(pass);
        }
    }

    // Invalidates itself it the pass does invalidate. e.g.: modifies the nodes
    // inside the module.
    if (pass->doesInvalidatesModule())
        invalidate();
}

std::unique_ptr<efd::QModule> efd::QModule::clone() const {
    NodeRef newAST = mAST->clone();
    return GetFromAST(newAST);
}

std::unique_ptr<efd::QModule> efd::QModule::GetFromAST(NodeRef ref) {
    std::unique_ptr<QModule> qmod(new QModule(ref));
    qmod->validate();
    return qmod;
}

std::unique_ptr<efd::QModule> efd::QModule::Parse(std::string filename, 
        std::string path, bool forceStdLib) {
    NodeRef ast = efd::ParseFile(filename, path, forceStdLib);

    if (ast != nullptr)
        return GetFromAST(ast);

    return std::unique_ptr<QModule>(nullptr);
}

std::unique_ptr<efd::QModule> efd::QModule::ParseString(std::string program, bool forceStdLib) {
    NodeRef ast = efd::ParseString(program, forceStdLib);

    if (ast != nullptr)
        return GetFromAST(ast);

    return std::unique_ptr<QModule>(nullptr);
}
