#include "enfield/Analysis/Driver.h"
#include "enfield/Transform/QModule.h"
#include "enfield/Transform/QModulefyPass.h"
#include "enfield/Transform/IdTable.h"
#include "enfield/Transform/Utils.h"
#include "enfield/Support/RTTI.h"
#include "enfield/Pass.h"

#include <cassert>
#include <iterator>

namespace efd {
    extern const efd::NodeRef SWAP_ID_NODE;
    extern const efd::NodeRef H_ID_NODE;
    extern const efd::NodeRef CX_ID_NODE;
}

efd::QModule::QModule(NodeRef ref) : mAST(ref), mVersion(nullptr), 
    mQModulefy(nullptr), mValid(true), mStdLibsParsed(false) {
}

void efd::QModule::registerSwapGate(Iterator it) {
    bool isSwapRegistered = getQGate("__swap__") != nullptr;
    if (!isSwapRegistered) {
        NDGateDecl* swap;

        // The quantum arguments that will be used
        NDId* qargLhs = dynCast<NDId>(NDId::Create("a"));
        NDId* qargRhs = dynCast<NDId>(NDId::Create("b"));

        NDList* qargsLhs = dynCast<NDList>(NDList::Create());
        NDList* qargsRhs = dynCast<NDList>(NDList::Create());
        NDList* qargsLhsRhs = dynCast<NDList>(NDList::Create());

        qargsLhs->addChild(qargLhs->clone());
        qargsRhs->addChild(qargRhs->clone());
        qargsLhsRhs->addChild(qargLhs->clone());
        qargsLhsRhs->addChild(qargRhs->clone());

        // The quantum operations
        NDGOpList* gop = dynCast<NDGOpList>(NDGOpList::Create());
        // cx a, b;
        gop->addChild(NDQOpGeneric::Create(CX_ID_NODE->clone(), NDList::Create(), 
                    qargsLhsRhs->clone()));
        // h a;
        gop->addChild(NDQOpGeneric::Create(H_ID_NODE->clone(), NDList::Create(), 
                    qargsLhs->clone()));
        // h b;
        gop->addChild(NDQOpGeneric::Create(H_ID_NODE->clone(), NDList::Create(), 
                    qargsRhs->clone()));
        // cx a, b;
        gop->addChild(NDQOpGeneric::Create(CX_ID_NODE->clone(), NDList::Create(), 
                    qargsLhsRhs->clone()));
        // h a;
        gop->addChild(NDQOpGeneric::Create(H_ID_NODE->clone(), NDList::Create(), 
                    qargsLhs->clone()));
        // h b;
        gop->addChild(NDQOpGeneric::Create(H_ID_NODE->clone(), NDList::Create(), 
                    qargsRhs->clone()));
        // cx a, b;
        gop->addChild(NDQOpGeneric::Create(CX_ID_NODE->clone(), NDList::Create(), 
                    qargsLhsRhs->clone()));

        swap = dynCast<NDGateDecl>(NDGateDecl::Create(SWAP_ID_NODE->clone(),
                    NDList::Create(), qargsLhsRhs, gop));

        // Inserts swap declaration before first use.
        insertNodeBefore(it, swap);
    }
}

efd::NodeRef efd::QModule::getVersion() {
    return mVersion;
}

void efd::QModule::replaceAllRegsWith(std::vector<NDDecl*> newRegs) {
    if (!mRegDecls.empty()) {
        NDStmtList* parent = dynCast<NDStmtList>(mRegDecls[0]->getParent());
        assert(parent != nullptr && 
                "All register declaration must be in the main statement list.");

        for (int i = mRegDecls.size() - 1; i >= 0; --i) {
            auto child = dynCast<NDDecl>(mRegDecls[i]);
            if (child->isQReg()) {
                auto it = parent->findChild(child);
                parent->removeChild(it);
            }
        }
    }

    assert(!mStatements.empty() &&
            "Trying to replace regs with no statements in the program.");
    NodeRef firstStmt = *stmt_begin();
    auto it = firstStmt->getParent()->findChild(firstStmt);
    for (auto decl : newRegs)
        InsertNodeBefore(it, decl);
    invalidate();
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

efd::QModule::Iterator efd::QModule::insertSwapBefore(Iterator it, NodeRef lhs, NodeRef rhs) {
    NodeRef parent = (*it)->getParent();
    unsigned dist = std::distance(parent->begin(), it);
    InsertSwapBefore(*it, lhs, rhs);
    // Register the gate must be done after inserting the swap, as it does not require
    // any iterator. Else, this would invalidate the iterator passed as parammeter.
    it = parent->begin() + dist;
    registerSwapGate(it);
    invalidate();
    return parent->begin() + dist;
}

efd::QModule::Iterator efd::QModule::insertSwapAfter(Iterator it, NodeRef lhs, NodeRef rhs) {
    NodeRef parent = (*it)->getParent();
    unsigned dist = std::distance(parent->begin(), it);
    InsertSwapAfter(*it, lhs, rhs);
    // Register the gate must be done after inserting the swap, as it does not require
    // any iterator. Else, this would invalidate the iterator passed as parammeter.
    registerSwapGate(it);
    invalidate();
    return parent->begin() + dist;
}

efd::QModule::Iterator efd::QModule::reg_begin() {
    if (!isValid()) validate();
    return mRegDecls.begin();
}

efd::QModule::ConstIterator efd::QModule::reg_begin() const {
    assert(isValid() && "Const QModule modified.");
    return mRegDecls.begin();
}

efd::QModule::Iterator efd::QModule::reg_end() {
    if (!isValid()) validate();
    return mRegDecls.end();
}

efd::QModule::ConstIterator efd::QModule::reg_end() const {
    assert(isValid() && "Const QModule modified.");
    return mRegDecls.end();
}

efd::QModule::Iterator efd::QModule::gates_begin() {
    if (!isValid()) validate();
    return mGates.begin();
}

efd::QModule::ConstIterator efd::QModule::gates_begin() const {
    assert(isValid() && "Const QModule modified.");
    return mGates.begin();
}

efd::QModule::Iterator efd::QModule::gates_end() {
    if (!isValid()) validate();
    return mGates.end();
}

efd::QModule::ConstIterator efd::QModule::gates_end() const {
    assert(isValid() && "Const QModule modified.");
    return mGates.end();
}

efd::QModule::Iterator efd::QModule::stmt_begin() {
    if (!isValid()) validate();
    return mStatements.begin();
}

efd::QModule::ConstIterator efd::QModule::stmt_begin() const {
    assert(isValid() && "Const QModule modified.");
    return mStatements.begin();
}

efd::QModule::Iterator efd::QModule::stmt_end() {
    if (!isValid()) validate();
    return mStatements.end();
}

efd::QModule::ConstIterator efd::QModule::stmt_end() const {
    assert(isValid() && "Const QModule modified.");
    return mStatements.end();
}

void efd::QModule::print(std::ostream& O, bool pretty) const {
    O << toString(pretty);
}

std::string efd::QModule::toString(bool pretty) const {
    return mAST->toString(pretty);
}

efd::IdTable& efd::QModule::getIdTable(NDGateDecl* ref) {
    if (!isValid()) validate();

    if (ref != nullptr && mIdTableMap.find(ref) != mIdTableMap.end())
        return mIdTableMap[ref];
    return mTable;
}

efd::NodeRef efd::QModule::getQVar(std::string id, NDGateDecl* gate, bool recursive) {
    if (!isValid()) validate();

    IdTable& gTable = getIdTable(gate);
    return gTable.getQVar(id, recursive);
}

efd::NDGateDecl* efd::QModule::getQGate(std::string id, bool recursive) {
    if (!isValid()) validate();
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

bool efd::QModule::isValid() const {
    return mValid;
}

void efd::QModule::runPass(Pass* pass, bool force) {
    if (pass->wasApplied() && !force)
        return;

    if (!isValid()) validate();

    pass->init(force);

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
    if (pass->doesInvalidatesModule()) {
        invalidate();
        validate();
    }
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
