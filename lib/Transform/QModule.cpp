#include "enfield/Analysis/Driver.h"
#include "enfield/Transform/QModule.h"
#include "enfield/Transform/Utils.h"
#include "enfield/Support/RTTI.h"
#include "enfield/Support/uRefCast.h"

#include <cassert>
#include <unordered_set>
#include <iterator>

namespace efd {
    extern NDId::uRef SWAP_ID_NODE;
    extern NDId::uRef H_ID_NODE;
    extern NDId::uRef CX_ID_NODE;
}

efd::QModule::QModule() : mVersion(nullptr) {
    mStatements = NDStmtList::Create();
}

void efd::QModule::registerSwapGate() {
    bool isSwapRegistered = hasQGate("__swap__");
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

        auto swap = NDGateDecl::Create
                (uniqueCastForward<NDId>(SWAP_ID_NODE->clone()),
                 NDList::Create(),
                 std::move(qargsLhsRhs),
                 std::move(gop));

        insertGate(std::move(swap));
    }
}

efd::NDQasmVersion::Ref efd::QModule::getVersion() {
    return mVersion.get();
}

void efd::QModule::setVersion(NDQasmVersion::uRef version) {
    mVersion = std::move(version);
}

void efd::QModule::insertInclude(NDInclude::uRef incl) {
    mIncludes.push_back(std::move(incl));
}

void efd::QModule::insertReg(NDRegDecl::uRef reg) {
    std::string id = reg->getId()->getVal();
    mRegsMap[id] = std::move(reg);
    mRegs.push_back(mRegsMap[id].get());
}

void efd::QModule::replaceAllRegsWith(std::vector<NDRegDecl::uRef> newRegs) {
    mRegs.clear();

    for (auto& reg : newRegs) {
        insertReg(std::move(reg));
    }
}

efd::QModule::Iterator efd::QModule::findStatement(Node::Ref ref) {
    auto it = mStatements->findChild(ref);
    assert(it != mStatements->end() && "Node not in the main statement list.");
    return it;
}

void efd::QModule::removeStatement(Iterator it) {
    mStatements->removeChild(it);
}

efd::QModule::Iterator efd::QModule::inlineCall(NDQOpGeneric::Ref call) {
    Node::Ref parent = call->getParent();
    Iterator it = parent->findChild(call);
    unsigned dist = std::distance(parent->begin(), it);

    InlineGate(this, call);
    return parent->begin() + dist;
}

efd::QModule::Iterator efd::QModule::insertStatementAfter(Iterator it, Node::uRef ref) {
    mStatements->addChild(++it, std::move(ref));
    return it;
}

efd::QModule::Iterator efd::QModule::insertStatementBefore(Iterator it, Node::uRef ref) {
    mStatements->addChild(it, std::move(ref));
    return it;
}

efd::QModule::Iterator efd::QModule::insertStatementFront(Node::uRef ref) {
    auto it = mStatements->begin();
    mStatements->addChild(it, std::move(ref));
    return mStatements->begin();
}

efd::QModule::Iterator efd::QModule::insertStatementLast(Node::uRef ref) {
    mStatements->addChild(std::move(ref));
    return mStatements->begin() + (mStatements->getChildNumber() - 1);
}

static efd::NDQOpGeneric::uRef createSwapCallNode(efd::Node::Ref lhs,
        efd::Node::Ref rhs) {
    auto qargs = efd::NDList::Create();
    qargs->addChild(lhs->clone());
    qargs->addChild(rhs->clone());

    auto swap = efd::NDQOpGeneric::Create(
            efd::uniqueCastForward<efd::NDId>(efd::SWAP_ID_NODE->clone()),
            efd::NDList::Create(), std::move(qargs));
    swap->setGenerated();
    return std::move(swap);
}

efd::QModule::Iterator efd::QModule::insertSwapBefore(Iterator it, Node::Ref lhs, Node::Ref rhs) {
    Node::Ref parent = (*it)->getParent();
    unsigned dist = std::distance(parent->begin(), it);
    insertStatementBefore(it, createSwapCallNode(lhs, rhs));
    registerSwapGate();
    return parent->begin() + dist;
}

efd::QModule::Iterator efd::QModule::insertSwapAfter(Iterator it, Node::Ref lhs, Node::Ref rhs) {
    Node::Ref parent = (*it)->getParent();
    unsigned dist = std::distance(parent->begin(), it);
    insertStatementAfter(it, createSwapCallNode(lhs, rhs));
    registerSwapGate();
    return parent->begin() + dist;
}

void efd::QModule::insertGate(NDGateSign::uRef gate) {
    assert(gate.get() != nullptr && "Trying to insert a 'nullptr' gate.");
    assert(gate->getId() != nullptr && "Trying to insert a gate with 'nullptr' id.");

    std::string id = gate->getId()->getVal();
    assert(mGatesMap.find(id) == mGatesMap.end() &&
            "Trying to insert a gate with repeated id.");

    mGatesMap[id] = std::move(gate);
    mGates.push_back(mGatesMap[id].get());
}

efd::QModule::RegIterator efd::QModule::reg_begin() {
    return mRegs.begin();
}

efd::QModule::RegConstIterator efd::QModule::reg_begin() const {
    return mRegs.begin();
}

efd::QModule::RegIterator efd::QModule::reg_end() {
    return mRegs.end();
}

efd::QModule::RegConstIterator efd::QModule::reg_end() const {
    return mRegs.end();
}

efd::QModule::GateIterator efd::QModule::gates_begin() {
    return mGates.begin();
}

efd::QModule::GateConstIterator efd::QModule::gates_begin() const {
    return mGates.begin();
}

efd::QModule::GateIterator efd::QModule::gates_end() {
    return mGates.end();
}

efd::QModule::GateConstIterator efd::QModule::gates_end() const {
    return mGates.end();
}

efd::QModule::Iterator efd::QModule::stmt_begin() {
    return mStatements->begin();
}

efd::QModule::ConstIterator efd::QModule::stmt_begin() const {
    return mStatements->begin();
}

efd::QModule::Iterator efd::QModule::stmt_end() {
    return mStatements->end();
}

efd::QModule::ConstIterator efd::QModule::stmt_end() const {
    return mStatements->end();
}

void efd::QModule::print(std::ostream& O, bool pretty, bool printGates) const {
    O << toString(pretty, printGates);
}

std::string efd::QModule::toString(bool pretty, bool printGates) const {
    std::string str;

    if (mVersion.get() != nullptr)
        str += mVersion->toString(pretty);

    for (auto& incl : mIncludes)
        str += incl->toString(pretty);

    if (printGates) {
        // Print all quantum gates.
        for (auto gate : mGates)
            str += gate->toString(pretty);
    } else {
        // Print those gates that are being used.
        std::unordered_set<std::string> doPrint;

        for (auto& stmt : *mStatements) {
            NDQOpGeneric::Ref qcall = nullptr;

            if (auto ifstmt = dynCast<NDIfStmt>(stmt.get())) {
                // Get the QOp part (if it is a NDIfStmt node).
                if (auto ifcall = dynCast<NDQOpGeneric>(ifstmt->getQOp())) {
                    qcall = ifcall;
                }
            } else {
                qcall = dynCast<NDQOpGeneric>(stmt.get());
            }

            if (qcall != nullptr) {
                std::string gateId = qcall->getId()->getVal();

                // Print if it was not yet printed.
                if (doPrint.find(gateId) == doPrint.end()) {
                    doPrint.insert(gateId);
                }
            }
        }

        for (auto gate : mGates) {
            if (doPrint.find(gate->getId()->getVal()) != doPrint.end() &&
                    !gate->isInInclude()) {
                str += gate->toString(pretty);
            }
        }
    }

    for (auto& reg : mRegs)
        str += reg->toString(pretty);

    str += mStatements->toString(pretty);
    return str;
}

efd::Node::Ref efd::QModule::getQVar(std::string id, NDGateDecl::Ref gate) const {
    if (gate != nullptr) {
        assert(mGateIdMap.find(gate) != mGateIdMap.end() && "No such gate found.");

        const IdMap& idMap = mGateIdMap.at(gate);
        assert(idMap.find(id) != idMap.end() && "No such id inside this gate.");

        return idMap.at(id);
    }

    // If gate == nullptr, then we want a quantum register in the global context.
    return mRegsMap.at(id).get();
}

bool efd::QModule::hasQVar(std::string id, NDGateDecl::Ref gate) const {
    if (gate != nullptr) {
        if (mGateIdMap.find(gate) == mGateIdMap.end()) return false;

        const IdMap& idMap = mGateIdMap.at(gate);
        if (idMap.find(id) == idMap.end()) return false;

        return true;
    }

    if (mRegsMap.find(id) == mRegsMap.end()) return false;
    // If gate == nullptr, then we want a quantum register in the global context.
    return true;
}

efd::NDGateSign::Ref efd::QModule::getQGate(std::string id) const {
    assert(mGatesMap.find(id) != mGatesMap.end() && "Gate not found.");
    return mGatesMap.at(id).get();
}

bool efd::QModule::hasQGate(std::string id) const {
    if (mGatesMap.find(id) == mGatesMap.end()) return false;
    return true;
}

efd::QModule::uRef efd::QModule::clone() const {
    auto qmod = new QModule();

    if (mVersion.get() != nullptr)
        qmod->mVersion = uniqueCastForward<NDQasmVersion>(mVersion->clone());

    for (auto& incl : mIncludes)
        qmod->insertInclude(uniqueCastForward<NDInclude>(incl->clone()));

    for (auto reg : mRegs)
        qmod->insertReg(uniqueCastForward<NDRegDecl>(reg->clone()));

    for (auto gate : mGates)
        qmod->insertGate(uniqueCastForward<NDGateSign>(gate->clone()));

    qmod->mStatements = uniqueCastForward<NDStmtList>(mStatements->clone());
    return uRef(qmod);
}

efd::QModule::uRef efd::QModule::Create(bool forceStdLib) {
    std::string program;
    program = "OPENQASM 2.0;\n";

    auto ast = efd::ParseString(program, forceStdLib);
    if (ast.get() != nullptr)
        return GetFromAST(std::move(ast));

    return uRef(nullptr);
}

efd::QModule::uRef efd::QModule::GetFromAST(Node::uRef ref) {
    uRef qmod(new QModule());
    efd::ProcessAST(qmod.get(), ref.get());
    return qmod;
}

efd::QModule::uRef efd::QModule::Parse(std::string filename, 
        std::string path, bool forceStdLib) {
    auto ast = efd::ParseFile(filename, path, forceStdLib);

    if (ast.get() != nullptr)
        return GetFromAST(std::move(ast));

    return uRef(nullptr);
}

efd::QModule::uRef efd::QModule::ParseString(std::string program, bool forceStdLib) {
    auto ast = efd::ParseString(program, forceStdLib);

    if (ast != nullptr)
        return GetFromAST(std::move(ast));

    return uRef(nullptr);
}
