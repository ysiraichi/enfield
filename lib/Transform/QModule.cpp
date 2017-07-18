#include "enfield/Analysis/Driver.h"
#include "enfield/Transform/QModule.h"
#include "enfield/Transform/QModulefyPass.h"
#include "enfield/Transform/IdTable.h"
#include "enfield/Transform/Utils.h"
#include "enfield/Support/RTTI.h"
#include "enfield/Support/uRefCast.h"
#include "enfield/Pass.h"

#include <cassert>
#include <iterator>

namespace efd {
    extern NDId::uRef SWAP_ID_NODE;
    extern NDId::uRef H_ID_NODE;
    extern NDId::uRef CX_ID_NODE;
}

efd::QModule::QModule(Node::uRef ref) : mAST(std::move(ref)), mVersion(nullptr), 
    mQModulefy(nullptr), mValid(true), mStdLibsParsed(false), mStmtList(nullptr) {
}

void efd::QModule::registerSwapGate(Iterator it) {
    bool isSwapRegistered = getQGate("__swap__") != nullptr;
    if (!isSwapRegistered) {
        // The quantum arguments that will be used
        auto qargLhs = NDId::Create("a");
        auto qargRhs = NDId::Create("b");

        auto qargsLhs = NDList::Create();
        auto qargsRhs = NDList::Create();
        auto qargsLhsRhs = NDList::Create();

        qargsLhs->addChild(qargLhs->clone());
        qargsRhs->addChild(qargRhs->clone());
        qargsLhsRhs->addChild(qargLhs->clone());
        qargsLhsRhs->addChild(qargRhs->clone());

        // The quantum operations
        auto gop = NDGOpList::Create();
        // cx a, b;
        gop->addChild(NDQOpGeneric::Create
                (uniqueCastForward<NDId>(CX_ID_NODE->clone()), NDList::Create(),
                 uniqueCastForward<NDList>(qargsLhsRhs->clone())));
        // h a;
        gop->addChild(NDQOpGeneric::Create
                (uniqueCastForward<NDId>(H_ID_NODE->clone()), NDList::Create(), 
                 uniqueCastForward<NDList>(qargsLhs->clone())));
        // h b;
        gop->addChild(NDQOpGeneric::Create
                (uniqueCastForward<NDId>(H_ID_NODE->clone()), NDList::Create(), 
                 uniqueCastForward<NDList>(qargsRhs->clone())));
        // cx a, b;
        gop->addChild(NDQOpGeneric::Create
                (uniqueCastForward<NDId>(CX_ID_NODE->clone()), NDList::Create(), 
                 uniqueCastForward<NDList>(qargsLhsRhs->clone())));
        // h a;
        gop->addChild(NDQOpGeneric::Create
                (uniqueCastForward<NDId>(H_ID_NODE->clone()), NDList::Create(),
                 uniqueCastForward<NDList>(qargsLhs->clone())));
        // h b;
        gop->addChild(NDQOpGeneric::Create
                (uniqueCastForward<NDId>(H_ID_NODE->clone()), NDList::Create(),
                 uniqueCastForward<NDList>(qargsRhs->clone())));
        // cx a, b;
        gop->addChild(NDQOpGeneric::Create
                (uniqueCastForward<NDId>(CX_ID_NODE->clone()), NDList::Create(),
                 uniqueCastForward<NDList>(qargsLhsRhs->clone())));

        auto swap = uniqueCastBackward<Node>(NDGateDecl::Create
                (uniqueCastForward<NDId>(SWAP_ID_NODE->clone()), NDList::Create(),
                 std::move(qargsLhsRhs), std::move(gop)));

        // Inserts swap declaration before first use.
        insertNodeBefore(it, std::move(swap));
    }
}

efd::Node::Ref efd::QModule::getVersion() {
    return mVersion;
}

void efd::QModule::replaceAllRegsWith(std::vector<NDDecl::uRef> newRegs) {
    if (!mRegDecls.empty()) {
        NDStmtList::Ref parent = dynCast<NDStmtList>(mRegDecls[0]->getParent());
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
    auto firstStmt = *stmt_begin();
    auto it = firstStmt->getParent()->findChild(firstStmt);
    for (auto& decl : newRegs)
        InsertNodeBefore(it, std::move(decl));
    invalidate();
}

efd::QModule::Iterator efd::QModule::inlineCall(NDQOpGeneric::Ref call) {
    Node::Ref parent = call->getParent();
    Iterator it = parent->findChild(call);
    unsigned dist = std::distance(parent->begin(), it);

    InlineGate(this, call);
    invalidate();
    return parent->begin() + dist;
}

efd::QModule::Iterator efd::QModule::insertNodeAfter(Iterator it, Node::uRef ref) {
    InsertNodeAfter(it, std::move(ref));
    invalidate();
    return it;
}

efd::QModule::Iterator efd::QModule::insertNodeBefore(Iterator it, Node::uRef ref) {
    InsertNodeBefore(it, std::move(ref));
    invalidate();
    return it;
}

efd::QModule::Iterator efd::QModule::insertSwapBefore(Iterator it, Node::Ref lhs, Node::Ref rhs) {
    Node::Ref parent = (*it)->getParent();
    unsigned dist = std::distance(parent->begin(), it);
    InsertSwapBefore(it->get(), lhs, rhs);
    // Register the gate must be done after inserting the swap, as it does not require
    // any iterator. Else, this would invalidate the iterator passed as parammeter.
    it = parent->begin() + dist;
    registerSwapGate(it);
    invalidate();
    return parent->begin() + dist;
}

efd::QModule::Iterator efd::QModule::insertSwapAfter(Iterator it, Node::Ref lhs, Node::Ref rhs) {
    Node::Ref parent = (*it)->getParent();
    unsigned dist = std::distance(parent->begin(), it);
    InsertSwapAfter(it->get(), lhs, rhs);
    // Register the gate must be done after inserting the swap, as it does not require
    // any iterator. Else, this would invalidate the iterator passed as parammeter.
    registerSwapGate(it);
    invalidate();
    return parent->begin() + dist;
}

void efd::QModule::insertStatementLast(Node::uRef node) {
    assert(mStmtList != nullptr && "Statement list is null.");
    mStmtList->addChild(std::move(node));
    invalidate();
}

efd::QModule::RegIterator efd::QModule::reg_begin() {
    if (!isValid()) validate();
    return mRegDecls.begin();
}

efd::QModule::RegConstIterator efd::QModule::reg_begin() const {
    assert(isValid() && "Const QModule modified.");
    return mRegDecls.begin();
}

efd::QModule::RegIterator efd::QModule::reg_end() {
    if (!isValid()) validate();
    return mRegDecls.end();
}

efd::QModule::RegConstIterator efd::QModule::reg_end() const {
    assert(isValid() && "Const QModule modified.");
    return mRegDecls.end();
}

efd::QModule::GateIterator efd::QModule::gates_begin() {
    if (!isValid()) validate();
    return mGates.begin();
}

efd::QModule::GateConstIterator efd::QModule::gates_begin() const {
    assert(isValid() && "Const QModule modified.");
    return mGates.begin();
}

efd::QModule::GateIterator efd::QModule::gates_end() {
    if (!isValid()) validate();
    return mGates.end();
}

efd::QModule::GateConstIterator efd::QModule::gates_end() const {
    assert(isValid() && "Const QModule modified.");
    return mGates.end();
}

efd::QModule::NodeIterator efd::QModule::stmt_begin() {
    if (!isValid()) validate();
    return mStatements.begin();
}

efd::QModule::NodeConstIterator efd::QModule::stmt_begin() const {
    assert(isValid() && "Const QModule modified.");
    return mStatements.begin();
}

efd::QModule::NodeIterator efd::QModule::stmt_end() {
    if (!isValid()) validate();
    return mStatements.end();
}

efd::QModule::NodeConstIterator efd::QModule::stmt_end() const {
    assert(isValid() && "Const QModule modified.");
    return mStatements.end();
}

void efd::QModule::print(std::ostream& O, bool pretty) const {
    O << toString(pretty);
}

std::string efd::QModule::toString(bool pretty) const {
    return mAST->toString(pretty);
}

efd::IdTable::Ref efd::QModule::getIdTable(NDGateDecl* ref) {
    if (!isValid()) validate();

    if (ref != nullptr && mIdTableMap.find(ref) != mIdTableMap.end())
        return mIdTableMap[ref].get();
    return mTable.get();
}

efd::Node::Ref efd::QModule::getQVar(std::string id, NDGateDecl* gate, bool recursive) {
    if (!isValid()) validate();

    IdTable::Ref gTable = getIdTable(gate);
    return gTable->getQVar(id, recursive);
}

efd::NDGateDecl* efd::QModule::getQGate(std::string id, bool recursive) {
    if (!isValid()) validate();
    return mTable->getQGate(id, recursive);
}

void efd::QModule::invalidate() {
    mValid = false;
}

void efd::QModule::validate() {
    if (mQModulefy.get() == nullptr)
        mQModulefy = toShared(QModulefyPass::Create(this));
    mQModulefy->setQModule(this);

    mVersion = nullptr;
    mStmtList = nullptr;
    mRegDecls.clear();
    mGates.clear();
    mStatements.clear();

    // mValid must be set before calling 'runPass'. Otherwise,
    // infinite loop!.
    mValid = true;
    runPass(mQModulefy.get(), true);
}

bool efd::QModule::isValid() const {
    return mValid;
}

void efd::QModule::runPass(Pass::Ref pass, bool force) {
    if (pass->wasApplied() && !force)
        return;

    if (!isValid()) validate();

    pass->init(force);

    if (pass->isASTPass()) {
        mAST->apply(pass);
    } else {
        if (pass->isRegDeclPass()) {
            auto regsCopy = mRegDecls;
            for (auto regdecl : regsCopy)
                regdecl->apply(pass);
        }
        
        if (pass->isGatePass()) {
            auto gatesCopy = mGates;
            for (auto gate : gatesCopy)
                gate->apply(pass);
        }

        if (pass->isStatementPass()) {
            auto stmtCopy = mStatements;
            for (auto stmt : stmtCopy)
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
    auto newAST = mAST->clone();
    return GetFromAST(std::move(newAST));
}

std::unique_ptr<efd::QModule> efd::QModule::Create(bool forceStdLib) {
    std::string program;
    program = "OPENQASM 2.0;\n";

    auto ast = efd::ParseString(program, forceStdLib);
    if (ast.get() != nullptr)
        return GetFromAST(std::move(ast));

    return uRef(nullptr);
}

std::unique_ptr<efd::QModule> efd::QModule::GetFromAST(Node::uRef ref) {
    uRef qmod(new QModule(std::move(ref)));
    qmod->validate();
    return qmod;
}

std::unique_ptr<efd::QModule> efd::QModule::Parse(std::string filename, 
        std::string path, bool forceStdLib) {
    auto ast = efd::ParseFile(filename, path, forceStdLib);

    if (ast.get() != nullptr)
        return GetFromAST(std::move(ast));

    return uRef(nullptr);
}

std::unique_ptr<efd::QModule> efd::QModule::ParseString(std::string program, bool forceStdLib) {
    auto ast = efd::ParseString(program, forceStdLib);

    if (ast != nullptr)
        return GetFromAST(std::move(ast));

    return uRef(nullptr);
}
