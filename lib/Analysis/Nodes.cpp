#include "enfield/Analysis/Nodes.h"
#include "enfield/Analysis/NodeVisitor.h"
#include "enfield/Support/RTTI.h"

#include <algorithm>
#include <cassert>

efd::Node::Node(Kind k, unsigned size, bool empty) : mK(k), mIsEmpty(empty),
    mChild(size), mWasGenerated(false) {
}

efd::Node::~Node() {
}

efd::Node::Ref efd::Node::getChild(unsigned i) const {
    return mChild[i].get();
}

void efd::Node::setChild(unsigned i, uRef ref) {
    ref->setParent(this);
    mChild[i] = std::move(ref);
}

efd::Node::Iterator efd::Node::findChild(Ref ref) {
    for (auto it = mChild.begin(), end = mChild.end(); it != end; ++it) {
        if (ref == it->get()) return it;
    }

    return mChild.end();
}

efd::Node::Iterator efd::Node::begin() {
    return mChild.begin();
}

efd::Node::Iterator efd::Node::end() {
    return mChild.end();
}

efd::Node::ConstIterator efd::Node::begin() const {
    return mChild.begin();
}

efd::Node::ConstIterator efd::Node::end() const {
    return mChild.end();
}

void efd::Node::print(std::ostream& O, bool pretty) {
    O << toString(pretty);
}

void efd::Node::print(bool pretty) {
    std::cout << toString(pretty);
}

bool efd::Node::isEmpty() const {
    return mIsEmpty;  
}

bool efd::Node::wasGenerated() const {
    return mWasGenerated;
}

void efd::Node::setGenerated() {
    mWasGenerated = true;
}

efd::Node::Ref efd::Node::getParent() const {
    return mParent;
}

void efd::Node::setParent(Ref ref) {
    mParent = ref;
}

std::string efd::Node::getOperation() const {
    return "";
}

unsigned efd::Node::getChildNumber() const {
    return 0;
}

void efd::Node::apply(NodeVisitor::Ref visitor) {
    applyImpl(visitor);
    for (auto& child : mChild)
        child->apply(visitor);
}

// -------------- Value Specializations -----------------
// -------------- Value<efd::IntVal> -----------------
template <> 
efd::NDValue<efd::IntVal>::NDValue(efd::IntVal val) 
    : Node(K_LIT_INT, getChildNumber()), mVal(val) {
}

template <> 
bool efd::NDValue<efd::IntVal>::ClassOf(const Node* node) { 
    return node->getKind() == K_LIT_INT; 
}

template <> 
efd::Node::Kind efd::NDValue<efd::IntVal>::getKind() const {
    return K_LIT_INT; 
}

template <> 
void efd::NDValue<efd::IntVal>::applyImpl(NodeVisitor::Ref visitor) {
    visitor->visit(this);
}

// -------------- Value<efd::RealVal> -----------------
template <> 
efd::NDValue<efd::RealVal>::NDValue(efd::RealVal val) : 
    Node(K_LIT_REAL), mVal(val) {
}

template <> 
bool efd::NDValue<efd::RealVal>::ClassOf(const Node* node) {
    return node->getKind() == K_LIT_REAL; 
}

template <> 
efd::Node::Kind efd::NDValue<efd::RealVal>::getKind() const { 
    return K_LIT_REAL; 
}

template <> 
void efd::NDValue<efd::RealVal>::applyImpl(NodeVisitor::Ref visitor) {
    visitor->visit(this);
}

// -------------- Value<std::string> -----------------
template <> 
efd::NDValue<std::string>::NDValue(std::string val) 
    : Node(K_LIT_STRING, getChildNumber()), mVal(val) {
}

template <> 
bool efd::NDValue<std::string>::ClassOf(const Node* node) {
    return node->getKind() == K_LIT_STRING; 
}

template <> 
efd::Node::Kind efd::NDValue<std::string>::getKind() const {
    return K_LIT_STRING; 
}

template <> 
std::string efd::NDValue<std::string>::getOperation() const {
    return mVal; 
}

template <> 
void efd::NDValue<std::string>::applyImpl(NodeVisitor::Ref visitor) {
    visitor->visit(this);
}

template <> 
std::string efd::NDValue<std::string>::toString(bool pretty) const {
    return mVal; 
}

// -------------- Decl -----------------
efd::NDDecl::NDDecl(Kind k, unsigned childNumber, NDId::uRef idNode) : Node(k, childNumber) {
    setId(std::move(idNode));
}

efd::NDId::Ref efd::NDDecl::getId() const {
    return dynCast<NDId>(mChild[I_ID].get());
}

void efd::NDDecl::setId(NDId::uRef ref) {
    setChild(I_ID, Node::uRef(ref.release()));
}

bool efd::NDDecl::isReg() const {
    return getKind() == K_REG_DECL;
}

bool efd::NDDecl::isGate() const {
    return getKind() == K_GATE_DECL;
}

bool efd::NDDecl::ClassOf(const Node* node) {
    return node->getKind() == K_REG_DECL ||
        node->getKind() == K_GATE_DECL;
}

// -------------- RegDecl -----------------
efd::NDRegDecl::NDRegDecl(Type t, NDId::uRef idNode, NDInt::uRef sizeNode) :
    NDDecl(K_REG_DECL, getChildNumber(), std::move(idNode)), mT(t) {
    setSize(std::move(sizeNode));
}

efd::Node::uRef efd::NDRegDecl::clone() const {
    auto id = dynCast<NDId>(getId()->clone().release());
    auto size = dynCast<NDInt>(getSize()->clone().release());
    return Node::uRef(NDRegDecl::Create(mT, NDId::uRef(id), NDInt::uRef(size)).release());
}

efd::NDInt::Ref efd::NDRegDecl::getSize() const {
    return dynCast<NDInt>(mChild[I_SIZE].get());
}

void efd::NDRegDecl::setSize(NDInt::uRef ref) {
    setChild(I_SIZE, Node::uRef(ref.release()));
}

bool efd::NDRegDecl::isCReg() const {
    return mT == CONCRETE;
}

bool efd::NDRegDecl::isQReg() const {
    return mT == QUANTUM;
}

std::string efd::NDRegDecl::getOperation() const {
    switch (mT) {
        case CONCRETE: return "creg";
        case QUANTUM:  return "qreg";
    }
}

unsigned efd::NDRegDecl::getChildNumber() const {
    return 2;
}

void efd::NDRegDecl::applyImpl(NodeVisitor::Ref visitor) {
    visitor->visit(this);
}

std::string efd::NDRegDecl::toString(bool pretty) const {
    std::string str;
    std::string endl = (pretty) ? "\n" : "";

    str += getOperation();
    str += " " + getId()->toString(pretty);
    str += "[" + getSize()->toString(pretty) + "];";
    str += endl;

    return str;
}

efd::Node::Kind efd::NDRegDecl::getKind() const {
    return K_REG_DECL;
}

bool efd::NDRegDecl::ClassOf(const Node* node) {
    return node->getKind() == K_REG_DECL;
}

efd::NDRegDecl::uRef efd::NDRegDecl::Create(Type t, NDId::uRef idNode, NDInt::uRef sizeNode) {
    return uRef(new NDRegDecl(t, std::move(idNode), std::move(sizeNode)));
}

efd::NDRegDecl::uRef efd::NDRegDecl::CreateQ(NDId::uRef idNode, NDInt::uRef sizeNode) {
    return Create(QUANTUM, std::move(idNode), std::move(sizeNode));
}

efd::NDRegDecl::uRef efd::NDRegDecl::CreateC(NDId::uRef idNode, NDInt::uRef sizeNode) {
    return Create(CONCRETE, std::move(idNode), std::move(sizeNode));
}

// -------------- ID reference Operation -----------------
efd::NDIdRef::NDIdRef(NDId::uRef idNode, NDInt::uRef nNode) : Node(K_ID_REF, getChildNumber()) {
    setId(std::move(idNode));
    setN(std::move(nNode));
}

efd::Node::uRef efd::NDIdRef::clone() const {
    auto id = dynCast<NDId>(getId()->clone().release());
    auto n = dynCast<NDInt>(getN()->clone().release());
    return Node::uRef(NDIdRef::Create(NDId::uRef(id), NDInt::uRef(n)).release());
}

efd::NDId::Ref efd::NDIdRef::getId() const {
    return dynCast<NDId>(mChild[I_ID].get());
}

void efd::NDIdRef::setId(NDId::uRef ref) {
    setChild(I_ID, Node::uRef(ref.release()));
}

efd::NDInt::Ref efd::NDIdRef::getN() const {
    return dynCast<NDInt>(mChild[I_N].get());
}

void efd::NDIdRef::setN(NDInt::uRef ref) {
    setChild(I_N, Node::uRef(ref.release()));
}

unsigned efd::NDIdRef::getChildNumber() const {
    return 2;
}

void efd::NDIdRef::applyImpl(NodeVisitor::Ref visitor) {
    visitor->visit(this);
}

std::string efd::NDIdRef::toString(bool pretty) const {
    std::string str;

    str += getId()->toString(pretty);
    str += "[" + getN()->toString(pretty) + "]";

    return str;
}

efd::Node::Kind efd::NDIdRef::getKind() const {
    return K_ID_REF;
}

bool efd::NDIdRef::ClassOf(const Node* node) {
    return node->getKind() == K_ID_REF;
}

efd::NDIdRef::uRef efd::NDIdRef::Create(NDId::uRef idNode, NDInt::uRef nNode) {
    return std::unique_ptr<NDIdRef>(new NDIdRef(std::move(idNode), std::move(nNode)));
}

// -------------- Node List -----------------
efd::NDList::NDList() : Node(K_LIST, 0, true) {
}

efd::NDList::NDList(Kind k, unsigned size) : Node(k, size, true) {
}

efd::NDList::~NDList() {
}

void efd::NDList::cloneChildremTo(NDList::Ref list) const {
    for (auto& child : *this) {
        list->addChild(child->clone());
    }
}

efd::Node::uRef efd::NDList::clone() const {
    NDList::Ref list = dynCast<NDList>(NDList::Create().release());
    cloneChildremTo(list);
    return Node::uRef(list);
}

void efd::NDList::addChild(Node::uRef child) {
    child->setParent(this);
    mChild.push_back(std::move(child));
    Node::mIsEmpty = false;
}

void efd::NDList::addChild(Iterator& It, Node::uRef child) {
    child->setParent(this);
    It = mChild.insert(It, std::move(child));
    Node::mIsEmpty = false;
}

void efd::NDList::removeChild(Iterator& It) {
    It = mChild.erase(It);
    if (mChild.empty())
        Node::mIsEmpty = true;
}

void efd::NDList::removeChild(Node::Ref ref) {
    auto It = findChild(ref);
    assert(It != end() && "Can't remove inexistent child.");

    removeChild(It);
    if (mChild.empty())
        Node::mIsEmpty = true;
}

unsigned efd::NDList::getChildNumber() const {
    return mChild.size();
}

void efd::NDList::applyImpl(NodeVisitor::Ref visitor) {
    visitor->visit(this);
}

std::string efd::NDList::toString(bool pretty) const {
    std::string str;

    if (!mChild.empty()) {
        str += getChild(0)->toString(pretty);

        for (auto it = mChild.begin() + 1, end = mChild.end(); it != end; ++it)
            str += ", " + (*it)->toString(pretty);
    }

    return str;
}

efd::Node::Kind efd::NDList::getKind() const {
    return K_LIST;
}

bool efd::NDList::ClassOf(const Node* node) {
    return node->getKind() == K_LIST ||
        node->getKind() == K_STMT_LIST ||
        node->getKind() == K_GOP_LIST;
}

efd::NDList::uRef efd::NDList::Create() {
    return uRef(new NDList());
}

// -------------- StmtList -----------------
efd::NDStmtList::NDStmtList() : NDList(K_STMT_LIST, 0) {
}

efd::Node::uRef efd::NDStmtList::clone() const {
    NDList::Ref list = dynCast<NDList>(NDStmtList::Create().release());
    cloneChildremTo(list);
    return Node::uRef(list);
}

void efd::NDStmtList::applyImpl(NodeVisitor::Ref visitor) {
    visitor->visit(this);
}

std::string efd::NDStmtList::toString(bool pretty) const {
    std::string str;

    for (auto &child : *this)
        str += child->toString(pretty);

    return str;
}

efd::Node::Kind efd::NDStmtList::getKind() const {
    return K_STMT_LIST;
}

bool efd::NDStmtList::ClassOf(const Node* node) {
    return node->getKind() == K_STMT_LIST; 
}

efd::NDStmtList::uRef efd::NDStmtList::Create() {
    return uRef(new NDStmtList());
}

// -------------- GOpList -----------------
efd::NDGOpList::NDGOpList() : NDList(K_GOP_LIST, 0) {
}

efd::Node::uRef efd::NDGOpList::clone() const {
    NDList::Ref list = dynCast<NDList>(NDGOpList::Create().release());
    cloneChildremTo(list);
    return Node::uRef(list);
}

void efd::NDGOpList::applyImpl(NodeVisitor::Ref visitor) {
    visitor->visit(this);
}

std::string efd::NDGOpList::toString(bool pretty) const {
    std::string str;
    std::string endl = (pretty) ? "\n" : "";
    std::string tab = (pretty) ? "\t" : "";

    for (auto &child : *this)
        if (!child->isEmpty())
            str += tab + child->toString(pretty);

    return str;
}

efd::Node::Kind efd::NDGOpList::getKind() const {
    return K_GOP_LIST;
}

bool efd::NDGOpList::ClassOf(const Node* node) {
    return node->getKind() == K_GOP_LIST; 
}

efd::NDGOpList::uRef efd::NDGOpList::Create() {
    return uRef(new NDGOpList());
}

// -------------- If Statement -----------------
efd::NDIfStmt::NDIfStmt(NDId::uRef cidNode, NDInt::uRef nNode, Node::uRef qopNode) :
    Node(K_IF_STMT, getChildNumber()) {
    setCondId(std::move(cidNode));
    setCondN(std::move(nNode));
    setQOp(std::move(qopNode));
}

efd::Node::uRef efd::NDIfStmt::clone() const {
    auto cid = dynCast<NDId>(getCondId()->clone().release());
    auto cn = dynCast<NDInt>(getCondN()->clone().release());
    auto qop = getQOp()->clone().release();
    return uRef(NDIfStmt::Create(NDId::uRef(cid), NDInt::uRef(cn), Node::uRef(qop)).release());
}

efd::NDId::Ref efd::NDIfStmt::getCondId() const {
    return dynCast<NDId>(mChild[I_COND_ID].get());
}

void efd::NDIfStmt::setCondId(NDId::uRef ref) {
    setChild(I_COND_ID, Node::uRef(ref.release()));
}

efd::NDInt::Ref efd::NDIfStmt::getCondN() const {
    return dynCast<NDInt>(mChild[I_COND_N].get());
}

void efd::NDIfStmt::setCondN(NDInt::uRef ref) {
    setChild(I_COND_N, Node::uRef(ref.release()));
}

efd::Node::Ref efd::NDIfStmt::getQOp() const {
    return mChild[I_QOP].get();
}

void efd::NDIfStmt::setQOp(Node::uRef ref) {
    setChild(I_QOP, Node::uRef(ref.release()));
}

unsigned efd::NDIfStmt::getChildNumber() const {
    return 3;
}

void efd::NDIfStmt::applyImpl(NodeVisitor::Ref visitor) {
    visitor->visit(this);
}

std::string efd::NDIfStmt::toString(bool pretty) const {
    std::string str;
    std::string endl = (pretty) ? "\n" : "";

    str += getOperation() + " ";

    str += "(";
    str += getCondId()->toString(pretty);
    str += " == ";
    str += getCondN()->toString(pretty);
    str += ")";

    str += " " + getQOp()->toString(pretty);

    return str;
}

std::string efd::NDIfStmt::getOperation() const {
    return "if";
}

efd::Node::Kind efd::NDIfStmt::getKind() const {
    return K_IF_STMT;
}

bool efd::NDIfStmt::ClassOf(const Node* node) {
    return node->getKind() == K_IF_STMT; 
}

efd::NDIfStmt::uRef efd::NDIfStmt::Create(NDId::uRef cidNode, NDInt::uRef nNode, Node::uRef qopNode) {
    return uRef(new NDIfStmt(std::move(cidNode), std::move(nNode), std::move(qopNode)));
}

// -------------- QasmVersion -----------------
efd::NDQasmVersion::NDQasmVersion(NDReal::uRef vNode, NDStmtList::uRef stmtsNode) :
    Node(K_QASM_VERSION, getChildNumber()) {
    setVersion(std::move(vNode));
    setStatements(std::move(stmtsNode));
}

efd::Node::uRef efd::NDQasmVersion::clone() const {
    auto v = dynCast<NDReal>(getVersion()->clone().release());
    auto stmt = dynCast<NDStmtList>(getStatements()->clone().release());
    return Node::uRef(NDQasmVersion::Create(NDReal::uRef(v), NDStmtList::uRef(stmt)).release());
}

std::string efd::NDQasmVersion::getOperation() const {
    return "OPENQASM";
}

unsigned efd::NDQasmVersion::getChildNumber() const {
    return 2;
}

void efd::NDQasmVersion::applyImpl(NodeVisitor::Ref visitor) {
    visitor->visit(this);
}

efd::NDReal::Ref efd::NDQasmVersion::getVersion() const {
    return dynCast<NDReal>(mChild[I_VERSION].get());
}

void efd::NDQasmVersion::setVersion(NDReal::uRef ref) {
    setChild(I_VERSION, Node::uRef(ref.release()));
}

efd::NDStmtList::Ref efd::NDQasmVersion::getStatements() const {
    return dynCast<NDStmtList>(mChild[I_STMTS].get());
}

void efd::NDQasmVersion::setStatements(NDStmtList::uRef ref) {
    setChild(I_STMTS, Node::uRef(ref.release()));
}

std::string efd::NDQasmVersion::toString(bool pretty) const {
    std::string str;
    std::string endl = (pretty) ? "\n" : "";

    str += getOperation();
    str += " " + getVersion()->toString(pretty) + ";";
    str += endl;

    str += getStatements()->toString(pretty);

    return str;
}

efd::Node::Kind efd::NDQasmVersion::getKind() const {
    return K_QASM_VERSION;
}

bool efd::NDQasmVersion::ClassOf(const Node* node) {
    return node->getKind() == K_QASM_VERSION;
}

efd::NDQasmVersion::uRef efd::NDQasmVersion::Create(NDReal::uRef vNode, NDStmtList::uRef stmtsNode) {
    return uRef(new NDQasmVersion(std::move(vNode), std::move(stmtsNode)));
}

// -------------- Include -----------------
efd::NDInclude::NDInclude(NDId::uRef fNode, Node::uRef astNode) :
    Node(K_INCLUDE, getChildNumber()) {
    setFilename(std::move(fNode));
    setInnerAST(std::move(astNode));
}

efd::Node::uRef efd::NDInclude::clone() const {
    auto f = dynCast<NDString>(getFilename()->clone().release());
    return Node::uRef(NDInclude::Create(NDString::uRef(f), getInnerAST()->clone()).release());
}

std::string efd::NDInclude::getOperation() const {
    return "include";
}

unsigned efd::NDInclude::getChildNumber() const {
    return 2;
}

void efd::NDInclude::applyImpl(NodeVisitor::Ref visitor) {
    visitor->visit(this);
}

efd::NDString::Ref efd::NDInclude::getFilename() const {
    return dynCast<NDString>(mChild[I_FILE].get());
}

void efd::NDInclude::setFilename(NDString::uRef ref) {
    setChild(I_FILE, Node::uRef(ref.release()));
}

efd::Node::Ref efd::NDInclude::getInnerAST() const {
    return dynCast<NDStmtList>(mChild[I_INNER_AST].get());
}

void efd::NDInclude::setInnerAST(Node::uRef ref) {
    setChild(I_INNER_AST, Node::uRef(ref.release()));
}

std::string efd::NDInclude::toString(bool pretty) const {
    std::string str;
    std::string endl = (pretty) ? "\n" : "";

    str += getOperation();
    str += " \"" + getFilename()->toString(pretty) + "\";";
    str += endl;

    return str;
}

efd::Node::Kind efd::NDInclude::getKind() const {
    return K_INCLUDE;
}

bool efd::NDInclude::ClassOf(const Node* node) {
    return node->getKind() == K_INCLUDE;
}

efd::NDInclude::uRef efd::NDInclude::Create(NDId::uRef fNode, Node::uRef astNode) {
    return uRef(new NDInclude(std::move(fNode), std::move(astNode)));
}

// -------------- Opaque -----------------
efd::NDGateSign::NDGateSign(Kind k, unsigned childNumber,
        NDId::uRef idNode, NDList::uRef aNode, NDList::uRef qaNode) :
    NDDecl(k, childNumber, std::move(idNode)) {
    setArgs(std::move(aNode));
    setQArgs(std::move(qaNode));
}

efd::NDGateSign::NDGateSign(NDId::uRef idNode, NDList::uRef aNode, NDList::uRef qaNode) :
    NDDecl(K_GATE_OPAQUE, getChildNumber(), std::move(idNode)) {
    setArgs(std::move(aNode));
    setQArgs(std::move(qaNode));
}

bool efd::NDGateSign::isOpaque() const {
    return getKind() == K_GATE_OPAQUE;
}

efd::Node::uRef efd::NDGateSign::clone() const {
    auto id = dynCast<NDId>(getId()->clone().release());
    auto args = dynCast<NDList>(getArgs()->clone().release());
    auto qargs = dynCast<NDList>(getQArgs()->clone().release());
    return Node::uRef(NDGateSign::Create(NDId::uRef(id), NDList::uRef(args), NDList::uRef(qargs)).release());
}

efd::NDList::Ref efd::NDGateSign::getArgs() const {
    return dynCast<NDList>(mChild[I_ARGS].get());
}

void efd::NDGateSign::setArgs(NDList::uRef ref) {
    setChild(I_ARGS, Node::uRef(ref.release()));
}

efd::NDList::Ref efd::NDGateSign::getQArgs() const {
    return dynCast<NDList>(mChild[I_QARGS].get());
}

void efd::NDGateSign::setQArgs(NDList::uRef ref) {
    setChild(I_QARGS, Node::uRef(ref.release()));
}

std::string efd::NDGateSign::getOperation() const {
    return "opaque";
}

unsigned efd::NDGateSign::getChildNumber() const {
    return 3;
}

void efd::NDGateSign::applyImpl(NodeVisitor::Ref visitor) {
    visitor->visit(this);
}

std::string efd::NDGateSign::toString(bool pretty) const {
    std::string str;
    std::string endl = (pretty) ? "\n" : "";

    str += getOperation();
    str += " " + getId()->toString(pretty);

    Node::Ref refArgs = getArgs();
    if (!refArgs->isEmpty())
        str += "(" + refArgs->toString(pretty) + ")";

    str += " " + getQArgs()->toString(pretty) + ";";
    str += endl;

    return str;
}

efd::Node::Kind efd::NDGateSign::getKind() const {
    return K_GATE_OPAQUE;
}

bool efd::NDGateSign::ClassOf(const Node* node) {
    return node->getKind() == K_GATE_OPAQUE;
}

efd::NDGateSign::uRef efd::NDGateSign::Create(NDId::uRef idNode, NDList::uRef aNode, NDList::uRef qaNode) {
    return uRef(new NDGateSign(std::move(idNode), std::move(aNode), std::move(qaNode)));
}

// -------------- GateDecl -----------------
efd::NDGateDecl::NDGateDecl(NDId::uRef idNode, NDList::uRef aNode, NDList::uRef qaNode,
        NDGOpList::uRef gopNode) : NDGateSign(K_GATE_DECL, getChildNumber(),
            std::move(idNode), std::move(aNode), std::move(qaNode)) {
    setGOpList(std::move(gopNode));
}

efd::Node::uRef efd::NDGateDecl::clone() const {
    auto id = dynCast<NDId>(getId()->clone().release());
    auto args = dynCast<NDList>(getArgs()->clone().release());
    auto qargs = dynCast<NDList>(getQArgs()->clone().release());
    auto gop = dynCast<NDGOpList>(getGOpList()->clone().release());
    return Node::uRef(NDGateDecl::Create(
                NDId::uRef(id), NDList::uRef(args),
                NDList::uRef(qargs), NDGOpList::uRef(gop)));
}

efd::NDGOpList::Ref efd::NDGateDecl::getGOpList() const {
    return dynCast<NDGOpList>(mChild[I_GOPLIST].get());
}

void efd::NDGateDecl::setGOpList(NDGOpList::uRef ref) {
    setChild(I_GOPLIST, Node::uRef(ref.release()));
}

std::string efd::NDGateDecl::getOperation() const {
    return "gate";
}

unsigned efd::NDGateDecl::getChildNumber() const {
    return 4;
}

void efd::NDGateDecl::applyImpl(NodeVisitor::Ref visitor) {
    visitor->visit(this);
}

std::string efd::NDGateDecl::toString(bool pretty) const {
    std::string str;
    std::string endl = (pretty) ? "\n" : "";

    str += getOperation();
    str += " " + getId()->toString(pretty);

    Node::Ref refArgs = getArgs();
    if (!refArgs->isEmpty())
        str += "(" + refArgs->toString(pretty) + ")";

    str += " " + getQArgs()->toString(pretty) + " {";
    str += endl;

    Node::Ref refGOpList = getGOpList();
    if (!refGOpList->isEmpty())
        str += refGOpList->toString(pretty);
    str += "}";
    str += endl;

    return str;
}

efd::Node::Kind efd::NDGateDecl::getKind() const {
    return K_GATE_DECL;
}

bool efd::NDGateDecl::ClassOf(const Node* node) {
    return node->getKind() == K_GATE_DECL;
}

efd::NDGateDecl::uRef efd::NDGateDecl::Create(NDId::uRef idNode, NDList::uRef aNode,
        NDList::uRef qaNode, NDGOpList::uRef gopNode) {
    return uRef(new NDGateDecl(
                std::move(idNode), std::move(aNode),
                std::move(qaNode), std::move(gopNode)));
}

// -------------- Qubit Operation -----------------
efd::NDQOp::NDQOp(Kind k, unsigned size) : Node(k, size) {
}

efd::NDQOp::~NDQOp() {
}

bool efd::NDQOp::isReset() const {
    return getKind() == K_QOP_RESET;
}

bool efd::NDQOp::isBarrier() const {
    return getKind() == K_QOP_BARRIER;
}

bool efd::NDQOp::isMeasure() const {
    return getKind() == K_QOP_MEASURE;
}

bool efd::NDQOp::isU() const {
    return getKind() == K_QOP_U;
}

bool efd::NDQOp::isCX() const {
    return getKind() == K_QOP_CX;
}

bool efd::NDQOp::isGeneric() const {
    return getKind() == K_QOP_GENERIC;
}

bool efd::NDQOp::ClassOf(const Node* node) {
    return node->getKind() == K_QOP_MEASURE ||
        node->getKind() == K_QOP_RESET ||
        node->getKind() == K_QOP_BARRIER ||
        node->getKind() == K_QOP_CX ||
        node->getKind() == K_QOP_U ||
        node->getKind() == K_QOP_GENERIC;
}

// -------------- Qubit Operation: Reset -----------------
efd::NDQOpReset::NDQOpReset(Node::uRef qaNode) : NDQOp(K_QOP_RESET, getChildNumber()) {
    setQArg(std::move(qaNode));
}

efd::Node::uRef efd::NDQOpReset::clone() const {
    return Node::uRef(NDQOpReset::Create(getQArg()->clone()).release());
}

efd::Node::Ref efd::NDQOpReset::getQArg() const {
    return mChild[I_ONLY].get();
}

void efd::NDQOpReset::setQArg(Node::uRef ref) {
    setChild(I_ONLY, Node::uRef(ref.release()));
}

std::string efd::NDQOpReset::getOperation() const {
    return "reset";
}

unsigned efd::NDQOpReset::getChildNumber() const {
    return 1;
}

void efd::NDQOpReset::applyImpl(NodeVisitor::Ref visitor) {
    visitor->visit(this);
}

std::string efd::NDQOpReset::toString(bool pretty) const {
    std::string str;
    std::string endl = (pretty) ? "\n" : "";

    str += getOperation();
    str += " " + getQArg()->toString(pretty) + ";";

    str += endl;

    return str;
}

efd::Node::Kind efd::NDQOpReset::getKind() const {
    return K_QOP_RESET;
}

bool efd::NDQOpReset::ClassOf(const Node* node) {
    return node->getKind() == K_QOP_RESET;
}

efd::NDQOpReset::uRef efd::NDQOpReset::Create(Node::uRef qaNode) {
    return uRef(new NDQOpReset(std::move(qaNode)));
}

// -------------- Qubit Operation: Barrier -----------------
efd::NDQOpBarrier::NDQOpBarrier(NDList::uRef qaNode) : NDQOp(K_QOP_BARRIER, getChildNumber()) {
    setQArgs(std::move(qaNode));
}

efd::Node::uRef efd::NDQOpBarrier::clone() const {
    auto qargs = dynCast<NDList>(getQArgs()->clone().release());
    return Node::uRef(NDQOpBarrier::Create(NDList::uRef(qargs)).release());
}

efd::NDList::Ref efd::NDQOpBarrier::getQArgs() const {
    return dynCast<NDList>(mChild[I_ONLY].get());
}

void efd::NDQOpBarrier::setQArgs(NDList::uRef ref) {
    setChild(I_ONLY, Node::uRef(ref.release()));
}

std::string efd::NDQOpBarrier::getOperation() const {
    return "barrier";
}

unsigned efd::NDQOpBarrier::getChildNumber() const {
    return 1;
}

void efd::NDQOpBarrier::applyImpl(NodeVisitor::Ref visitor) {
    visitor->visit(this);
}

std::string efd::NDQOpBarrier::toString(bool pretty) const {
    std::string str;
    std::string endl = (pretty) ? "\n" : "";

    str += getOperation();
    str += " " + getQArgs()->toString(pretty) + ";";

    str += endl;

    return str;
}

efd::Node::Kind efd::NDQOpBarrier::getKind() const {
    return K_QOP_BARRIER;
}

bool efd::NDQOpBarrier::ClassOf(const Node* node) {
    return node->getKind() == K_QOP_BARRIER;
}

efd::NDQOpBarrier::uRef efd::NDQOpBarrier::Create(NDList::uRef qaNode) {
    return uRef(new NDQOpBarrier(std::move(qaNode)));
}

// -------------- Qubit Operation: Measure -----------------
efd::NDQOpMeasure::NDQOpMeasure(Node::uRef qNode, Node::uRef cNode) : NDQOp(K_QOP_MEASURE, getChildNumber()) {
    setQBit(std::move(qNode));
    setCBit(std::move(cNode));
}

efd::Node::uRef efd::NDQOpMeasure::clone() const {
    return Node::uRef(NDQOpMeasure::Create(getQBit()->clone(), getCBit()->clone()).release());
}

efd::Node::Ref efd::NDQOpMeasure::getQBit() const {
    return mChild[I_QBIT].get();
}

void efd::NDQOpMeasure::setQBit(Node::uRef ref) {
    setChild(I_QBIT, Node::uRef(ref.release()));
}

efd::Node::Ref efd::NDQOpMeasure::getCBit() const {
    return mChild[I_CBIT].get();
}

void efd::NDQOpMeasure::setCBit(Node::uRef ref) {
    setChild(I_CBIT, Node::uRef(ref.release()));
}

std::string efd::NDQOpMeasure::getOperation() const {
    return "measure";
}

unsigned efd::NDQOpMeasure::getChildNumber() const {
    return 2;
}

void efd::NDQOpMeasure::applyImpl(NodeVisitor::Ref visitor) {
    visitor->visit(this);
}

std::string efd::NDQOpMeasure::toString(bool pretty) const {
    std::string str;
    std::string endl = (pretty) ? "\n" : "";

    str += getOperation();

    str += " " + getQBit()->toString(pretty);
    str += " ->";
    str += " " + getCBit()->toString(pretty) + ";";

    str += endl;

    return str;
}

efd::Node::Kind efd::NDQOpMeasure::getKind() const {
    return K_QOP_MEASURE;
}

bool efd::NDQOpMeasure::ClassOf(const Node* node) {
    return node->getKind() == K_QOP_MEASURE;
}

efd::NDQOpMeasure::uRef efd::NDQOpMeasure::Create(Node::uRef qNode, Node::uRef cNode) {
    return uRef(new NDQOpMeasure(std::move(qNode), std::move(cNode)));
}

// -------------- Qubit Operation: U -----------------
efd::NDQOpU::NDQOpU(NDList::uRef aNode, Node::uRef qaNode) : NDQOp(K_QOP_U, getChildNumber()) {
    setArgs(std::move(aNode));
    setQArg(std::move(qaNode));
}

efd::Node::uRef efd::NDQOpU::clone() const {
    auto args = dynCast<NDList>(getArgs()->clone().release());
    return Node::uRef(NDQOpU::Create(NDList::uRef(args), getQArg()->clone()).release());
}

efd::NDList::Ref efd::NDQOpU::getArgs() const {
    return dynCast<NDList>(mChild[I_ARGS].get());
}

void efd::NDQOpU::setArgs(NDList::uRef ref) {
    setChild(I_ARGS, Node::uRef(ref.release()));
}

efd::Node::Ref efd::NDQOpU::getQArg() const {
    return mChild[I_QARG].get();
}

void efd::NDQOpU::setQArg(Node::uRef ref) {
    setChild(I_QARG, Node::uRef(ref.release()));
}

std::string efd::NDQOpU::getOperation() const {
    return "U";
}

unsigned efd::NDQOpU::getChildNumber() const {
    return 2;
}

void efd::NDQOpU::applyImpl(NodeVisitor::Ref visitor) {
    visitor->visit(this);
}

std::string efd::NDQOpU::toString(bool pretty) const {
    std::string str;
    std::string endl = (pretty) ? "\n" : "";

    str += getOperation();
    str += "(" + getArgs()->toString(pretty) + ")";
    str += " " + getQArg()->toString(pretty) + ";";

    str += endl;

    return str;
}

efd::Node::Kind efd::NDQOpU::getKind() const {
    return K_QOP_U;
}

bool efd::NDQOpU::ClassOf(const Node* node) {
    return node->getKind() == K_QOP_U;
}

efd::NDQOpU::uRef efd::NDQOpU::Create(NDList::uRef aNode, Node::uRef qaNode) {
    return uRef(new NDQOpU(std::move(aNode), std::move(qaNode)));
}

// -------------- Qubit Operation: CX -----------------
efd::NDQOpCX::NDQOpCX(Node::uRef lhsNode, Node::uRef rhsNode) : NDQOp(K_QOP_CX, getChildNumber()) {
    setLhs(std::move(lhsNode));
    setRhs(std::move(rhsNode));
}

efd::Node::uRef efd::NDQOpCX::clone() const {
    return Node::uRef(NDQOpCX::Create(getLhs()->clone(), getRhs()->clone()).release());
}

efd::Node::Ref efd::NDQOpCX::getLhs() const {
    return mChild[I_LHS].get();
}

void efd::NDQOpCX::setLhs(Node::uRef ref) {
    setChild(I_LHS, Node::uRef(ref.release()));
}

efd::Node::Ref efd::NDQOpCX::getRhs() const {
    return mChild[I_RHS].get();
}

void efd::NDQOpCX::setRhs(Node::uRef ref) {
    setChild(I_RHS, Node::uRef(ref.release()));
}

std::string efd::NDQOpCX::getOperation() const {
    return "CX";
}

unsigned efd::NDQOpCX::getChildNumber() const {
    return 2;
}

void efd::NDQOpCX::applyImpl(NodeVisitor::Ref visitor) {
    visitor->visit(this);
}

std::string efd::NDQOpCX::toString(bool pretty) const {
    std::string str;
    std::string endl = (pretty) ? "\n" : "";

    str += getOperation();
    str += " " + getLhs()->toString(pretty) + ",";
    str += " " + getRhs()->toString(pretty) + ";";

    str += endl;

    return str;
}

efd::Node::Kind efd::NDQOpCX::getKind() const {
    return K_QOP_CX;
}

bool efd::NDQOpCX::ClassOf(const Node* node) {
    return node->getKind() == K_QOP_CX;
}

efd::NDQOpCX::uRef efd::NDQOpCX::Create(Node::uRef lhsNode, Node::uRef rhsNode) {
    return uRef(new NDQOpCX(std::move(lhsNode), std::move(rhsNode)));
}

// -------------- Qubit Operation: Generic -----------------
efd::NDQOpGeneric::NDQOpGeneric(NDId::uRef idNode, NDList::uRef aNode, NDList::uRef qaNode) : 
    NDQOp(K_QOP_GENERIC, getChildNumber()) {
    setId(std::move(idNode));
    setArgs(std::move(aNode));
    setQArgs(std::move(qaNode));
}

efd::Node::uRef efd::NDQOpGeneric::clone() const {
    auto id = dynCast<NDId>(getId()->clone().release());
    auto args = dynCast<NDList>(getArgs()->clone().release());
    auto qargs = dynCast<NDList>(getQArgs()->clone().release());
    return Node::uRef(NDQOpGeneric::Create(NDId::uRef(id), NDList::uRef(args), NDList::uRef(qargs)).release());
}

efd::NDId::Ref efd::NDQOpGeneric::getId() const {
    return dynCast<NDId>(mChild[I_ID].get());
}

void efd::NDQOpGeneric::setId(NDId::uRef ref) {
    setChild(I_ID, Node::uRef(ref.release()));
}

efd::NDList::Ref efd::NDQOpGeneric::getArgs() const {
    return dynCast<NDList>(mChild[I_ARGS].get());
}

void efd::NDQOpGeneric::setArgs(NDList::uRef ref) {
    setChild(I_ARGS, Node::uRef(ref.release()));
}

efd::NDList::Ref efd::NDQOpGeneric::getQArgs() const {
    return dynCast<NDList>(mChild[I_QARGS].get());
}

void efd::NDQOpGeneric::setQArgs(NDList::uRef ref) {
    setChild(I_QARGS, Node::uRef(ref.release()));
}

std::string efd::NDQOpGeneric::getOperation() const {
    return getId()->toString();
}

unsigned efd::NDQOpGeneric::getChildNumber() const {
    return 3;
}

void efd::NDQOpGeneric::applyImpl(NodeVisitor::Ref visitor) {
    visitor->visit(this);
}

std::string efd::NDQOpGeneric::toString(bool pretty) const {
    std::string str;
    std::string endl = (pretty) ? "\n" : "";

    str += getOperation();

    Node::Ref refArgs = getArgs();
    if (!refArgs->isEmpty())
        str += "(" + refArgs->toString(pretty) + ")";

    str += " " + getQArgs()->toString(pretty) + ";";
    str += endl;

    return str;
}

efd::Node::Kind efd::NDQOpGeneric::getKind() const {
    return K_QOP_GENERIC;
}

bool efd::NDQOpGeneric::ClassOf(const Node* node) {
    return node->getKind() == K_QOP_GENERIC;
}

efd::NDQOpGeneric::uRef efd::NDQOpGeneric::Create(NDId::uRef idNode, NDList::uRef aNode, NDList::uRef qaNode) {
    return uRef(new NDQOpGeneric(std::move(idNode), std::move(aNode), std::move(qaNode)));
}

// -------------- Binary Operation -----------------
efd::NDBinOp::NDBinOp(OpType t, Node::uRef lhsNode, Node::uRef rhsNode) :
    Node(K_BINOP, getChildNumber()), mT(t) {
    setLhs(std::move(lhsNode));
    setRhs(std::move(rhsNode));
}

efd::Node::uRef efd::NDBinOp::clone() const {
    return Node::uRef(NDBinOp::Create(mT, getLhs()->clone(), getRhs()->clone()).release());
}

efd::Node::Ref efd::NDBinOp::getLhs() const {
    return mChild[I_LHS].get();
}

void efd::NDBinOp::setLhs(Node::uRef ref) {
    setChild(I_LHS, Node::uRef(ref.release()));
}

efd::Node::Ref efd::NDBinOp::getRhs() const {
    return mChild[I_RHS].get();
}

void efd::NDBinOp::setRhs(Node::uRef ref) {
    setChild(I_RHS, Node::uRef(ref.release()));
}

efd::NDBinOp::OpType efd::NDBinOp::getOpType() const {
    return mT;
}

bool efd::NDBinOp::isAdd() const {
    return mT == OP_ADD;
}

bool efd::NDBinOp::isSub() const {
    return mT == OP_SUB;
}

bool efd::NDBinOp::isMul() const {
    return mT == OP_MUL;
}

bool efd::NDBinOp::isDiv() const {
    return mT == OP_DIV;
}

bool efd::NDBinOp::isPow() const {
    return mT == OP_POW;
}

std::string efd::NDBinOp::getOperation() const {
    switch (mT) {
        case OP_ADD: return "+";
        case OP_SUB: return "-";
        case OP_MUL: return "*";
        case OP_DIV: return "/";
        case OP_POW: return "^";
    }
}

unsigned efd::NDBinOp::getChildNumber() const {
    return 2;
}

void efd::NDBinOp::applyImpl(NodeVisitor::Ref visitor) {
    visitor->visit(this);
}

std::string efd::NDBinOp::toString(bool pretty) const {
    std::string str;

    str += "(";
    str += getLhs()->toString(pretty);
    str += " " + getOperation() + " ";
    str += getRhs()->toString(pretty);
    str += ")";

    return str;
}

efd::Node::Kind efd::NDBinOp::getKind() const {
    return K_BINOP;
}

bool efd::NDBinOp::ClassOf(const Node* node) {
    return node->getKind() == K_BINOP;
}

efd::NDBinOp::uRef efd::NDBinOp::Create(OpType t, Node::uRef lhsNode, Node::uRef rhsNode) {
    return uRef(new NDBinOp(t, std::move(lhsNode), std::move(rhsNode)));
}

efd::NDBinOp::uRef efd::NDBinOp::CreateAdd(Node::uRef lhsNode, Node::uRef rhsNode) {
    return Create(OP_ADD, std::move(lhsNode), std::move(rhsNode));
}

efd::NDBinOp::uRef efd::NDBinOp::CreateSub(Node::uRef lhsNode, Node::uRef rhsNode) {
    return Create(OP_SUB, std::move(lhsNode), std::move(rhsNode));
}

efd::NDBinOp::uRef efd::NDBinOp::CreateMul(Node::uRef lhsNode, Node::uRef rhsNode) {
    return Create(OP_MUL, std::move(lhsNode), std::move(rhsNode));
}

efd::NDBinOp::uRef efd::NDBinOp::CreateDiv(Node::uRef lhsNode, Node::uRef rhsNode) {
    return Create(OP_DIV, std::move(lhsNode), std::move(rhsNode));
}

efd::NDBinOp::uRef efd::NDBinOp::CreatePow(Node::uRef lhsNode, Node::uRef rhsNode) {
    return Create(OP_POW, std::move(lhsNode), std::move(rhsNode));
}

// -------------- Unary Operation -----------------
efd::NDUnaryOp::NDUnaryOp(UOpType t, Node::uRef oNode) : Node(K_UNARYOP, getChildNumber()), mT(t) {
    setOperand(std::move(oNode));
}

efd::Node::uRef efd::NDUnaryOp::clone() const {
    return Node::uRef(NDUnaryOp::Create(mT, getOperand()->clone()).release());
}

efd::Node::Ref efd::NDUnaryOp::getOperand() const {
    return mChild[I_ONLY].get();
}

void efd::NDUnaryOp::setOperand(Node::uRef ref) {
    setChild(I_ONLY, Node::uRef(ref.release()));
}

efd::NDUnaryOp::UOpType efd::NDUnaryOp::getUOpType() const {
    return mT;
}

bool efd::NDUnaryOp::isSin() const {
    return mT == UOP_SIN;
}

bool efd::NDUnaryOp::isCos() const {
    return mT == UOP_COS;
}

bool efd::NDUnaryOp::isTan() const {
    return mT == UOP_TAN;
}

bool efd::NDUnaryOp::isExp() const {
    return mT == UOP_EXP;
}

bool efd::NDUnaryOp::isLn() const {
    return mT == UOP_LN;
}

bool efd::NDUnaryOp::isSqrt() const {
    return mT == UOP_SQRT;
}

bool efd::NDUnaryOp::isNeg() const {
    return mT == UOP_NEG;
}

std::string efd::NDUnaryOp::getOperation() const {
    switch (mT) {
        case UOP_SIN:   return "sin";
        case UOP_COS:   return "cos";
        case UOP_TAN:   return "tan";
        case UOP_EXP:   return "exp";
        case UOP_LN:    return "ln";
        case UOP_SQRT:  return "sqrt";
        case UOP_NEG:   return "-";
    }
}

unsigned efd::NDUnaryOp::getChildNumber() const {
    return 1;
}

void efd::NDUnaryOp::applyImpl(NodeVisitor::Ref visitor) {
    visitor->visit(this);
}

std::string efd::NDUnaryOp::toString(bool pretty) const {
    std::string str;

    Node::Ref refOperand = getOperand();
    if (mT == UOP_NEG) {
        str += "(";
        str += getOperation();
        str += refOperand->toString(pretty);
        str += ")";
    } else {
        str += getOperation();
        str += "(" + refOperand->toString(pretty) + ")";
    }

    return str;
}

efd::Node::Kind efd::NDUnaryOp::getKind() const {
    return K_UNARYOP;
}

bool efd::NDUnaryOp::ClassOf(const Node* node) {
    return node->getKind() == K_UNARYOP;
}

efd::NDUnaryOp::uRef efd::NDUnaryOp::Create(UOpType t, Node::uRef oNode) {
    return uRef(new NDUnaryOp(t, std::move(oNode)));
}

efd::NDUnaryOp::uRef efd::NDUnaryOp::CreateNeg(Node::uRef oNode) {
    return uRef(new NDUnaryOp(UOP_NEG, std::move(oNode)));
}

efd::NDUnaryOp::uRef efd::NDUnaryOp::CreateSin(Node::uRef oNode) {
    return uRef(new NDUnaryOp(UOP_SIN, std::move(oNode)));
}

efd::NDUnaryOp::uRef efd::NDUnaryOp::CreateCos(Node::uRef oNode) {
    return uRef(new NDUnaryOp(UOP_COS, std::move(oNode)));
}

efd::NDUnaryOp::uRef efd::NDUnaryOp::CreateTan(Node::uRef oNode) {
    return uRef(new NDUnaryOp(UOP_TAN, std::move(oNode)));
}

efd::NDUnaryOp::uRef efd::NDUnaryOp::CreateExp(Node::uRef oNode) {
    return uRef(new NDUnaryOp(UOP_EXP, std::move(oNode)));
}

efd::NDUnaryOp::uRef efd::NDUnaryOp::CreateLn(Node::uRef oNode) {
    return uRef(new NDUnaryOp(UOP_LN, std::move(oNode)));
}

efd::NDUnaryOp::uRef efd::NDUnaryOp::CreateSqrt(Node::uRef oNode) {
    return uRef(new NDUnaryOp(UOP_SQRT, std::move(oNode)));
}
