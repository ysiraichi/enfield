#include "enfield/Analysis/Driver.h"
#include "enfield/Transform/QModule.h"
#include "enfield/Transform/Utils.h"
#include "enfield/Transform/PassCache.h"
#include "enfield/Support/RTTI.h"
#include "enfield/Support/uRefCast.h"
#include "enfield/Support/Defs.h"

#include <unordered_set>
#include <iterator>

efd::QModule::QModule() : mVersion(nullptr) {
    mStatements = NDStmtList::Create();
}

efd::QModule::~QModule() {
    PassCache::Clear(this);
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

void efd::QModule::removeAllQRegs() {
    std::vector<uint32_t> ridx;

    for (uint32_t i = 0, e = mRegs.size(); i < e; ++i) {
        if (mRegs[i]->isQReg())
            ridx.push_back(i);
    }

    // Starting from the end. So that we don't need to update the indexes
    // for every removal.
    for (auto it = ridx.rbegin(), e = ridx.rend(); it != e; ++it) {
        mRegs.erase(mRegs.begin() + *it);
    }
}

efd::QModule::Iterator efd::QModule::findStatement(Node::Ref ref) {
    auto it = mStatements->findChild(ref);

    if (it == mStatements->end()) {
        std::string nodeStr = (ref == nullptr) ? "nullptr" : ref->toString(false);
        ERR << "Node not in the main statement list: `" << nodeStr << "`." << std::endl;
        ExitWith(ExitCode::EXIT_unreachable);
    }

    return it;
}

void efd::QModule::removeStatement(Iterator it) {
    mStatements->removeChild(it);
}

efd::QModule::Iterator efd::QModule::inlineCall(NDQOp::Ref call) {
    if (!call->isGeneric()) {
        ERR << "Trying to inline a non-generic call: `" << call->toString(false)
            << "`." << std::endl;
        ExitWith(ExitCode::EXIT_unreachable);
    }

    Node::Ref stmt = call;
    auto parent = call->getParent();

    if (instanceOf<NDIfStmt>(parent)) {
        stmt = parent;
        parent = stmt->getParent();
    }

    Iterator it = parent->findChild(stmt);
    uint32_t dist = std::distance(parent->begin(), it);

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

efd::QModule::Iterator efd::QModule::replaceStatement
(Node::Ref stmt, std::vector<Node::uRef> stmts) {
    auto it = mStatements->findChild(stmt);

    if (it == mStatements->end()) {
        std::string stmtStr = (stmt == nullptr) ? "nullptr" : stmt->toString(false);
        ERR << "Trying to replace a non-existing statement: `" << stmtStr << "`." << std::endl;
        ExitWith(ExitCode::EXIT_unreachable);
    }

    uint32_t stmtsSize = stmts.size();
    if (!stmts.empty()) {
        it = mStatements->addChildren(it, std::move(stmts));
    }

    it = mStatements->removeChild(it + stmtsSize);
    return it - stmtsSize;
}

void efd::QModule::clearStatements() {
    mStatements->clear();
}

efd::Node::Ref efd::QModule::getStatement(uint32_t i) {
    if (i >= mStatements->getChildNumber()) {
        ERR << "Out of bounds access (of `" << mStatements->getChildNumber()
            << "`): `" << i << "`." << std::endl;
        ExitWith(ExitCode::EXIT_unreachable);
    }
    return mStatements->getChild(i);
}

void efd::QModule::insertGate(NDGateSign::uRef gate) {
    if (gate.get() == nullptr) {
        ERR << "Trying to insert a 'nullptr' gate." << std::endl;
        ExitWith(ExitCode::EXIT_unreachable);
    }

    if (gate->getId() == nullptr) {
        ERR << "Trying to insert a gate with 'nullptr' id." << std::endl;
        ExitWith(ExitCode::EXIT_unreachable);
    }

    std::string id = gate->getId()->getVal();

    if (mGatesMap.find(id) != mGatesMap.end()) {
        std::cerr << "Replacing gate: '" << id << "'." << std::endl;

        for (auto it = mGates.begin(), end = mGates.end(); it != end; ++it) {
            if ((*it)->getId()->getVal() == id) {
                mGates.erase(it);
                break;
            }
        }

        mGatesMap.erase(mGatesMap.find(id));
    }

    mGatesMap[id] = std::move(gate);
    mGates.push_back(mGatesMap[id].get());
}

uint32_t efd::QModule::getNumberOfRegs() const {
    return mRegs.size();
}

uint32_t efd::QModule::getNumberOfGates() const {
    return mGates.size();
}

uint32_t efd::QModule::getNumberOfStmts() const {
    return mStatements->getChildNumber();
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

void efd::QModule::orderby(std::vector<uint32_t> order) {
    std::vector<Node::uRef> newstmts;
    for (uint32_t i : order)
        newstmts.push_back(std::move(mStatements->mChild[i]));
    mStatements->mChild.clear();
    mStatements->mChild = std::move(newstmts);
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
            NDQOp::Ref qcall = nullptr;

            if (auto ifstmt = dynCast<NDIfStmt>(stmt.get())) {
                // Get the QOp part (if it is a NDIfStmt node).
                if (ifstmt->getQOp()->isGeneric()) {
                    qcall = ifstmt->getQOp();
                }
            } else {
                qcall = dynCast<NDQOp>(stmt.get());
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
        if (mGateIdMap.find(gate) == mGateIdMap.end()) {
            ERR << "No such gate found: `" << gate->getId()->getVal() << "`." << std::endl;
            ExitWith(ExitCode::EXIT_unknown_resource);
        }

        const IdMap& idMap = mGateIdMap.at(gate);
        if (idMap.find(id) == idMap.end()) {
            ERR << "No such id inside this gate (`" << gate->getId()->getVal()
                << "`): `" << id << "`." << std::endl;
            ExitWith(ExitCode::EXIT_unknown_resource);
        }

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
    if (mGatesMap.find(id) == mGatesMap.end()) {
        ERR << "Gate not found: `" << id << "`." << std::endl;
        ExitWith(ExitCode::EXIT_unknown_resource);
    }
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

efd::QModule::uRef efd::QModule::Create() {
    std::string program;
    program = "OPENQASM 2.0;\n";

    auto ast = efd::ParseString(program, true);
    if (ast.get() != nullptr)
        return GetFromAST(std::move(ast));

    return uRef(nullptr);
}

efd::QModule::uRef efd::QModule::GetFromAST(Node::uRef ref) {
    uRef qmod(new QModule());
    efd::ProcessAST(qmod.get(), ref.get());

    auto gates = efd::GetIntrinsicGates();
    for (auto& gate : gates)
        qmod->insertGate(std::move(gate));

    return qmod;
}

efd::QModule::uRef efd::QModule::Parse(std::string filename, std::string path) {
    auto ast = efd::ParseFile(filename, path, true);

    if (ast.get() != nullptr)
        return GetFromAST(std::move(ast));

    return uRef(nullptr);
}

efd::QModule::uRef efd::QModule::ParseString(std::string program) {
    auto ast = efd::ParseString(program, true);

    if (ast != nullptr)
        return GetFromAST(std::move(ast));

    return uRef(nullptr);
}
