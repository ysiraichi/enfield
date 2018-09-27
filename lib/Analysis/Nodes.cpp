#include "enfield/Analysis/Nodes.h"
#include "enfield/Analysis/NodeVisitor.h"
#include "enfield/Support/RTTI.h"
#include "enfield/Support/uRefCast.h"
#include "enfield/Support/Defs.h"

#include <algorithm>

efd::Node::Node(Kind k, bool empty) : mK(k), mIsEmpty(empty), mWasGenerated(false),
    mInInclude(false) {
}

void efd::Node::innerAddChild(uRef ref) {
    ref->setParent(this);
    mChild.push_back(std::move(ref));
}

efd::Node::~Node() {
}

efd::Node::Ref efd::Node::getChild(uint32_t i) const {
    return mChild[i].get();
}

void efd::Node::setChild(uint32_t i, uRef ref) {
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

efd::Node::Kind efd::Node::getKind() const {
    return mK;
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

bool efd::Node::isInInclude() const {
    return mInInclude;
}

void efd::Node::setInInclude() {
    mInInclude = true;
}

efd::Node::Ref efd::Node::getParent() const {
    return mParent;
}

void efd::Node::setParent(Ref ref) {
    mParent = ref;
}

bool efd::Node::equalsImpl(Node::Ref ref) const { return true; }

bool efd::Node::equals(Node::Ref ref) const {
    if (mK != ref->getKind()) return false;
    if (mChild.size() != ref->getChildNumber()) return false;
    if (getOperation() != ref->getOperation()) return false;

    uint32_t childNumber = getChildNumber();
    for (uint32_t i = 0; i < childNumber; ++i) {
        if (!mChild[i]->equals(ref->getChild(i)))
            return false;
    }

    return equalsImpl(ref);
}

std::string efd::Node::getOperation() const {
    return "";
}

efd::Node::uRef efd::Node::clone() const {
    auto cloned = cloneImpl();

    cloned->mK = mK;
    cloned->mIsEmpty = mIsEmpty;
    cloned->mInInclude = mInInclude;
    cloned->mWasGenerated = mWasGenerated;

    return cloned;
}

// -------------- Value Specializations -----------------
// -------------- Value<efd::IntVal> -----------------
template <> 
efd::NDValue<efd::IntVal>::NDValue(efd::IntVal val) : Node(K_LIT_INT), mVal(val) {
}

template <> 
bool efd::NDValue<efd::IntVal>::ClassOf(const Node* node) { 
    return node->getKind() == K_LIT_INT; 
}

template <> 
void efd::NDValue<efd::IntVal>::apply(NodeVisitor::Ref visitor) {
    visitor->visit(this);
}

// -------------- Value<efd::RealVal> -----------------
template <> 
efd::NDValue<efd::RealVal>::NDValue(efd::RealVal val) : Node(K_LIT_REAL), mVal(val) {
}

template <> 
bool efd::NDValue<efd::RealVal>::ClassOf(const Node* node) {
    return node->getKind() == K_LIT_REAL; 
}

template <> 
void efd::NDValue<efd::RealVal>::apply(NodeVisitor::Ref visitor) {
    visitor->visit(this);
}

// -------------- Value<std::string> -----------------
template <> 
efd::NDValue<std::string>::NDValue(std::string val) : Node(K_LIT_STRING), mVal(val) {
}

template <> 
bool efd::NDValue<std::string>::ClassOf(const Node* node) {
    return node->getKind() == K_LIT_STRING; 
}

template <> 
std::string efd::NDValue<std::string>::getOperation() const {
    return mVal; 
}

template <> 
void efd::NDValue<std::string>::apply(NodeVisitor::Ref visitor) {
    visitor->visit(this);
}

template <> 
std::string efd::NDValue<std::string>::toString(bool pretty) const {
    return mVal; 
}

// -------------- Decl -----------------
efd::NDDecl::NDDecl(Kind k, NDId::uRef idNode) : Node(k) {
    innerAddChild(std::move(idNode));
}

efd::NDId::Ref efd::NDDecl::getId() const {
    return dynCast<NDId>(mChild[I_ID].get());
}

void efd::NDDecl::setId(NDId::uRef ref) {
    setChild(I_ID, std::move(ref));
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
efd::NDRegDecl::NDRegDecl(Type t, NDId::uRef idNode, NDInt::uRef sizeNode)
                        : NDDecl(K_REG_DECL, std::move(idNode)), mT(t) {
    innerAddChild(std::move(sizeNode));
}

efd::Node::uRef efd::NDRegDecl::cloneImpl() const {
    auto id = uniqueCastForward<NDId>(getId()->clone());
    auto size = uniqueCastForward<NDInt>(getSize()->clone());
    return NDRegDecl::Create(mT, std::move(id), std::move(size));
}

efd::NDInt::Ref efd::NDRegDecl::getSize() const {
    return dynCast<NDInt>(mChild[I_SIZE].get());
}

void efd::NDRegDecl::setSize(NDInt::uRef ref) {
    setChild(I_SIZE, std::move(ref));
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
        default: EfdAbortIf(true, "Unknown register type.");
    }
}

uint32_t efd::NDRegDecl::getChildNumber() const {
    return 2;
}

void efd::NDRegDecl::apply(NodeVisitor::Ref visitor) {
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
efd::NDIdRef::NDIdRef(NDId::uRef idNode, NDInt::uRef nNode) : Node(K_ID_REF) {
    innerAddChild(std::move(idNode));
    innerAddChild(std::move(nNode));
}

efd::Node::uRef efd::NDIdRef::cloneImpl() const {
    auto id = uniqueCastForward<NDId>(getId()->clone());
    auto n = uniqueCastForward<NDInt>(getN()->clone());
    return NDIdRef::Create(std::move(id), std::move(n));
}

efd::NDId::Ref efd::NDIdRef::getId() const {
    return dynCast<NDId>(mChild[I_ID].get());
}

void efd::NDIdRef::setId(NDId::uRef ref) {
    setChild(I_ID, std::move(ref));
}

efd::NDInt::Ref efd::NDIdRef::getN() const {
    return dynCast<NDInt>(mChild[I_N].get());
}

void efd::NDIdRef::setN(NDInt::uRef ref) {
    setChild(I_N, std::move(ref));
}

uint32_t efd::NDIdRef::getChildNumber() const {
    return 2;
}

void efd::NDIdRef::apply(NodeVisitor::Ref visitor) {
    visitor->visit(this);
}

std::string efd::NDIdRef::toString(bool pretty) const {
    std::string str;

    str += getId()->toString(pretty);
    str += "[" + getN()->toString(pretty) + "]";

    return str;
}

bool efd::NDIdRef::ClassOf(const Node* node) {
    return node->getKind() == K_ID_REF;
}

efd::NDIdRef::uRef efd::NDIdRef::Create(NDId::uRef idNode, NDInt::uRef nNode) {
    return std::unique_ptr<NDIdRef>(new NDIdRef(std::move(idNode), std::move(nNode)));
}

// -------------- Node List -----------------
efd::NDList::NDList(Kind k) : Node(k, true) {
}

efd::NDList::~NDList() {
}

void efd::NDList::cloneChildrem(const NDList* list) {
    for (auto& child : *list)
        addChild(child->clone());
}

efd::Node::uRef efd::NDList::cloneImpl() const {
    auto list = NDList::Create(mK);
    list->cloneChildrem(this);
    return Node::uRef(list.release());
}

efd::Node::Iterator efd::NDList::addChild(Node::uRef child) {
    child->setParent(this);
    mChild.push_back(std::move(child));
    Node::mIsEmpty = false;
    return mChild.begin() + (mChild.size() - 1);
}

efd::Node::Iterator efd::NDList::addChild(Iterator it, Node::uRef child) {
    child->setParent(this);
    it = mChild.insert(it, std::move(child));
    Node::mIsEmpty = false;
    return it;
}

efd::Node::Iterator efd::NDList::addChildren(std::vector<Node::uRef> children) {
    mChild.reserve(mChild.size() + children.size());
    auto it = addChildren(mChild.end(), std::move(children));
    return it;
}

efd::Node::Iterator efd::NDList::addChildren
(Iterator it, std::vector<Node::uRef> children) {
    if (children.empty()) return mChild.end();

    for (auto& child : children)
        child->setParent(this);

    it = mChild.insert(it,
            std::make_move_iterator(children.begin()),
            std::make_move_iterator(children.end()));

    Node::mIsEmpty = false;
    return it;
}

efd::Node::Iterator efd::NDList::removeChild(Iterator it) {
    it = mChild.erase(it);
    if (mChild.empty())
        Node::mIsEmpty = true;
    return it;
}

void efd::NDList::clear() {
    mChild.clear();
}

void efd::NDList::removeChild(Node::Ref ref) {
    auto it = findChild(ref);
    EfdAbortIf(it == end(), "Can't remove inexistent child.");

    removeChild(it);
    if (mChild.empty())
        Node::mIsEmpty = true;
}

uint32_t efd::NDList::getChildNumber() const {
    return mChild.size();
}

void efd::NDList::apply(NodeVisitor::Ref visitor) {
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

bool efd::NDList::ClassOf(const Node* node) {
    return node->getKind() == K_LIST ||
        node->getKind() == K_STMT_LIST ||
        node->getKind() == K_GOP_LIST;
}

efd::NDList::uRef efd::NDList::Create(Kind k) {
    auto list = uRef(new NDList(k));
    return list;
}

efd::NDList::uRef efd::NDList::Create() {
    return Create(K_LIST);
}

// -------------- StmtList -----------------
efd::NDStmtList::NDStmtList() : NDList(K_STMT_LIST) {
}

efd::Node::uRef efd::NDStmtList::cloneImpl() const {
    auto list = NDStmtList::Create();
    list->cloneChildrem(this);
    return Node::uRef(list.release());
}

void efd::NDStmtList::apply(NodeVisitor::Ref visitor) {
    visitor->visit(this);
}

std::string efd::NDStmtList::toString(bool pretty) const {
    std::string str;

    for (auto &child : *this)
        str += child->toString(pretty);

    return str;
}

bool efd::NDStmtList::ClassOf(const Node* node) {
    return node->getKind() == K_STMT_LIST; 
}

efd::NDStmtList::uRef efd::NDStmtList::Create() {
    return uRef(new NDStmtList());
}

// -------------- GOpList -----------------
efd::NDGOpList::NDGOpList() : NDList(K_GOP_LIST) {
}

efd::Node::uRef efd::NDGOpList::cloneImpl() const {
    auto list = NDGOpList::Create();
    list->cloneChildrem(this);
    return Node::uRef(list.release());
}

void efd::NDGOpList::apply(NodeVisitor::Ref visitor) {
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


bool efd::NDGOpList::ClassOf(const Node* node) {
    return node->getKind() == K_GOP_LIST; 
}

efd::NDGOpList::uRef efd::NDGOpList::Create() {
    return uRef(new NDGOpList());
}

// -------------- If Statement -----------------
efd::NDIfStmt::NDIfStmt(NDId::uRef cidNode, NDInt::uRef nNode, NDQOp::uRef qopNode)
                      : Node(K_IF_STMT) {
    innerAddChild(std::move(cidNode));
    innerAddChild(std::move(nNode));
    innerAddChild(std::move(qopNode));
}

efd::Node::uRef efd::NDIfStmt::cloneImpl() const {
    auto cid = uniqueCastForward<NDId>(getCondId()->clone());
    auto cn = uniqueCastForward<NDInt>(getCondN()->clone());
    auto qop = uniqueCastForward<NDQOp>(getQOp()->clone());
    return NDIfStmt::Create(std::move(cid), std::move(cn), std::move(qop));
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

efd::NDQOp::Ref efd::NDIfStmt::getQOp() const {
    return dynCast<NDQOp>(mChild[I_QOP].get());
}

void efd::NDIfStmt::setQOp(NDQOp::uRef ref) {
    setChild(I_QOP, std::move(ref));
}

uint32_t efd::NDIfStmt::getChildNumber() const {
    return 3;
}

void efd::NDIfStmt::apply(NodeVisitor::Ref visitor) {
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

bool efd::NDIfStmt::ClassOf(const Node* node) {
    return node->getKind() == K_IF_STMT; 
}

efd::NDIfStmt::uRef efd::NDIfStmt::Create(NDId::uRef cidNode, NDInt::uRef nNode,
        NDQOp::uRef qopNode) {
    return uRef(new NDIfStmt(std::move(cidNode), std::move(nNode), std::move(qopNode)));
}

// -------------- QasmVersion -----------------
efd::NDQasmVersion::NDQasmVersion(NDReal::uRef vNode, NDStmtList::uRef stmtsNode) :
    Node(K_QASM_VERSION) {
    innerAddChild(std::move(vNode));
    innerAddChild(std::move(stmtsNode));
}

efd::Node::uRef efd::NDQasmVersion::cloneImpl() const {
    auto v = uniqueCastForward<NDReal>(getVersion()->clone());
    auto stmt = uniqueCastForward<NDStmtList>(getStatements()->clone());
    return NDQasmVersion::Create(std::move(v), std::move(stmt));
}

std::string efd::NDQasmVersion::getOperation() const {
    return "OPENQASM";
}

uint32_t efd::NDQasmVersion::getChildNumber() const {
    return 2;
}

void efd::NDQasmVersion::apply(NodeVisitor::Ref visitor) {
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

bool efd::NDQasmVersion::ClassOf(const Node* node) {
    return node->getKind() == K_QASM_VERSION;
}

efd::NDQasmVersion::uRef efd::NDQasmVersion::Create(NDReal::uRef vNode, NDStmtList::uRef stmtsNode) {
    return uRef(new NDQasmVersion(std::move(vNode), std::move(stmtsNode)));
}

// -------------- Include -----------------
efd::NDInclude::NDInclude(NDId::uRef fNode, Node::uRef astNode) :
    Node(K_INCLUDE) {
        innerAddChild(std::move(fNode));
        innerAddChild(std::move(astNode));
}

efd::Node::uRef efd::NDInclude::cloneImpl() const {
    auto f = uniqueCastForward<NDString>(getFilename()->clone());
    return NDInclude::Create(std::move(f), getInnerAST()->clone());
}

std::string efd::NDInclude::getOperation() const {
    return "include";
}

uint32_t efd::NDInclude::getChildNumber() const {
    return 2;
}

void efd::NDInclude::apply(NodeVisitor::Ref visitor) {
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

bool efd::NDInclude::ClassOf(const Node* node) {
    return node->getKind() == K_INCLUDE;
}

efd::NDInclude::uRef efd::NDInclude::Create(NDId::uRef fNode, Node::uRef astNode) {
    return uRef(new NDInclude(std::move(fNode), std::move(astNode)));
}

// -------------- Opaque -----------------
efd::NDGateSign::NDGateSign(Kind k, NDId::uRef idNode, NDList::uRef aNode, 
                            NDList::uRef qaNode) : NDDecl(k, std::move(idNode)) {
    innerAddChild(std::move(aNode));
    innerAddChild(std::move(qaNode));
}

efd::NDGateSign::NDGateSign(NDId::uRef idNode, NDList::uRef aNode, NDList::uRef qaNode) :
    NDDecl(K_GATE_OPAQUE, std::move(idNode)) {
    innerAddChild(std::move(aNode));
    innerAddChild(std::move(qaNode));
}

bool efd::NDGateSign::isOpaque() const {
    return getKind() == K_GATE_OPAQUE;
}

efd::Node::uRef efd::NDGateSign::cloneImpl() const {
    auto id = uniqueCastForward<NDId>(getId()->clone());
    auto args = uniqueCastForward<NDList>(getArgs()->clone());
    auto qargs = uniqueCastForward<NDList>(getQArgs()->clone());
    return NDGateSign::Create(std::move(id), std::move(args), std::move(qargs));
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

uint32_t efd::NDGateSign::getChildNumber() const {
    return 3;
}

void efd::NDGateSign::apply(NodeVisitor::Ref visitor) {
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

bool efd::NDGateSign::ClassOf(const Node* node) {
    return node->getKind() == K_GATE_OPAQUE ||
        node->getKind() == K_GATE_DECL;
}

efd::NDGateSign::uRef efd::NDGateSign::Create(NDId::uRef idNode, NDList::uRef aNode, NDList::uRef qaNode) {
    return uRef(new NDGateSign(std::move(idNode), std::move(aNode), std::move(qaNode)));
}

// -------------- GateDecl -----------------
efd::NDGateDecl::NDGateDecl(NDId::uRef idNode, NDList::uRef aNode, NDList::uRef qaNode,
        NDGOpList::uRef gopNode) : NDGateSign(K_GATE_DECL, std::move(idNode), 
            std::move(aNode), std::move(qaNode)) {
    innerAddChild(std::move(gopNode));
}

efd::Node::uRef efd::NDGateDecl::cloneImpl() const {
    auto id = uniqueCastForward<NDId>(getId()->clone());
    auto args = uniqueCastForward<NDList>(getArgs()->clone());
    auto qargs = uniqueCastForward<NDList>(getQArgs()->clone());
    auto gop = uniqueCastForward<NDGOpList>(getGOpList()->clone());
    return NDGateDecl::Create(std::move(id), std::move(args),
                std::move(qargs), std::move(gop));
}

efd::NDGOpList::Ref efd::NDGateDecl::getGOpList() const {
    return dynCast<NDGOpList>(mChild[I_GOPLIST].get());
}

void efd::NDGateDecl::setGOpList(NDGOpList::uRef ref) {
    setChild(I_GOPLIST, std::move(ref));
}

std::string efd::NDGateDecl::getOperation() const {
    return "gate";
}

uint32_t efd::NDGateDecl::getChildNumber() const {
    return 4;
}

void efd::NDGateDecl::apply(NodeVisitor::Ref visitor) {
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
efd::NDQOp::NDQOp(Kind k, NDId::uRef idNode, NDList::uRef aNode, NDList::uRef qaNode)
                : Node(k) {
    innerAddChild(std::move(idNode));
    innerAddChild(std::move(aNode));
    innerAddChild(std::move(qaNode));
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
    return getKind() == K_QOP_GEN;
}

efd::NDId::Ref efd::NDQOp::getId() const {
    return dynCast<NDId>(mChild[I_ID].get());
}

void efd::NDQOp::setId(NDId::uRef ref) {
    setChild(I_ID, std::move(ref));
}

efd::NDList::Ref efd::NDQOp::getArgs() const {
    return dynCast<NDList>(mChild[I_ARGS].get());
}

void efd::NDQOp::setArgs(NDList::uRef ref) {
    setChild(I_ARGS, std::move(ref));
}

efd::NDList::Ref efd::NDQOp::getQArgs() const {
    return dynCast<NDList>(mChild[I_QARGS].get());
}

void efd::NDQOp::setQArgs(NDList::uRef ref) {
    setChild(I_QARGS, std::move(ref));
}

std::string efd::NDQOp::getOperation() const {
    return getId()->toString();
}

uint32_t efd::NDQOp::getChildNumber() const {
    return 3;
}

std::string efd::NDQOp::toString(bool pretty) const {
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

bool efd::NDQOp::ClassOf(const Node* node) {
    auto kind = node->getKind();
    return
        kind == K_QOP_GEN     ||
        kind == K_QOP_U       ||
        kind == K_QOP_CX      ||
        kind == K_QOP_RESET   ||
        kind == K_QOP_BARRIER ||
        kind == K_QOP_MEASURE;
}

// -------------- Qubit Operation: Reset -----------------
efd::NDQOpReset::NDQOpReset(Node::uRef qaNode) : NDQOp(K_QOP_RESET,
        NDId::Create("reset"),
        NDList::Create(),
        NDList::Create()) {
    getQArgs()->addChild(std::move(qaNode));
}

efd::Node::Ref efd::NDQOpReset::getQArg() const {
    return mChild[I_QARGS]->getChild(I_ONLY);
}

void efd::NDQOpReset::setQArg(Node::uRef ref) {
    mChild[I_QARGS]->setChild(I_ONLY, std::move(ref));
}

void efd::NDQOpReset::apply(NodeVisitor::Ref visitor) {
    visitor->visit(this);
}

efd::Node::uRef efd::NDQOpReset::cloneImpl() const {
    return NDQOpReset::Create(getQArg()->clone());
}

bool efd::NDQOpReset::ClassOf(const Node* node) {
    return node->getKind() == K_QOP_RESET;
}

efd::NDQOpReset::uRef efd::NDQOpReset::Create(Node::uRef qaNode) {
    return uRef(new NDQOpReset(std::move(qaNode)));
}

// -------------- Qubit Operation: Barrier -----------------
efd::NDQOpBarrier::NDQOpBarrier(NDList::uRef qaNode) : NDQOp(K_QOP_BARRIER,
        NDId::Create("barrier"), NDList::Create(), std::move(qaNode)) {
}

void efd::NDQOpBarrier::apply(NodeVisitor::Ref visitor) {
    visitor->visit(this);
}

efd::Node::uRef efd::NDQOpBarrier::cloneImpl() const {
    auto qargs = uniqueCastForward<NDList>(getQArgs()->clone());
    return NDQOpBarrier::Create(std::move(qargs));
}

bool efd::NDQOpBarrier::ClassOf(const Node* node) {
    return node->getKind() == K_QOP_BARRIER;
}

efd::NDQOpBarrier::uRef efd::NDQOpBarrier::Create(NDList::uRef qaNode) {
    return uRef(new NDQOpBarrier(std::move(qaNode)));
}

// -------------- Qubit Operation: Measure -----------------
efd::NDQOpMeasure::NDQOpMeasure(Node::uRef qNode, Node::uRef cNode) :
    NDQOp(K_QOP_MEASURE, NDId::Create("measure"), NDList::Create(), NDList::Create()) {
    innerAddChild(std::move(cNode));
    getQArgs()->addChild(std::move(qNode));
}

efd::Node::uRef efd::NDQOpMeasure::cloneImpl() const {
    return NDQOpMeasure::Create(getQBit()->clone(), getCBit()->clone());
}

efd::Node::Ref efd::NDQOpMeasure::getQBit() const {
    return mChild[I_QARGS]->getChild(I_QBIT);
}

void efd::NDQOpMeasure::setQBit(Node::uRef ref) {
    mChild[I_QARGS]->setChild(I_QBIT, std::move(ref));
}

efd::Node::Ref efd::NDQOpMeasure::getCBit() const {
    return mChild[I_CBIT].get();
}

void efd::NDQOpMeasure::setCBit(Node::uRef ref) {
    setChild(I_CBIT, std::move(ref));
}

void efd::NDQOpMeasure::apply(NodeVisitor::Ref visitor) {
    visitor->visit(this);
}

uint32_t efd::NDQOpMeasure::getChildNumber() const {
    return 4;
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

bool efd::NDQOpMeasure::ClassOf(const Node* node) {
    return node->getKind() == K_QOP_MEASURE;
}

efd::NDQOpMeasure::uRef efd::NDQOpMeasure::Create(Node::uRef qNode, Node::uRef cNode) {
    return uRef(new NDQOpMeasure(std::move(qNode), std::move(cNode)));
}

// -------------- Qubit Operation: U -----------------
efd::NDQOpU::NDQOpU(NDList::uRef aNode, Node::uRef qaNode) : NDQOp(K_QOP_U,
        NDId::Create("U"), std::move(aNode), NDList::Create()) {
    getQArgs()->addChild(std::move(qaNode));
}

efd::Node::Ref efd::NDQOpU::getQArg() const {
    return mChild[I_QARGS]->getChild(I_ONLY);
}

void efd::NDQOpU::setQArg(Node::uRef ref) {
    mChild[I_QARGS]->setChild(I_ONLY, std::move(ref));
}

void efd::NDQOpU::apply(NodeVisitor::Ref visitor) {
    visitor->visit(this);
}

efd::Node::uRef efd::NDQOpU::cloneImpl() const {
    auto args = uniqueCastForward<NDList>(getArgs()->clone());
    return NDQOpU::Create(std::move(args), getQArg()->clone());
}

bool efd::NDQOpU::ClassOf(const Node* node) {
    return node->getKind() == K_QOP_U;
}

efd::NDQOpU::uRef efd::NDQOpU::Create(NDList::uRef aNode, Node::uRef qaNode) {
    return uRef(new NDQOpU(std::move(aNode), std::move(qaNode)));
}

// -------------- Qubit Operation: CX -----------------
efd::NDQOpCX::NDQOpCX(Node::uRef lhsNode, Node::uRef rhsNode) : NDQOp(K_QOP_CX,
        NDId::Create("CX"), NDList::Create(), NDList::Create()) {
    getQArgs()->addChild(std::move(lhsNode));
    getQArgs()->addChild(std::move(rhsNode));
}

efd::Node::Ref efd::NDQOpCX::getLhs() const {
    return mChild[I_QARGS]->getChild(I_LHS);
}

void efd::NDQOpCX::setLhs(Node::uRef ref) {
    mChild[I_QARGS]->setChild(I_LHS, std::move(ref));
}

efd::Node::Ref efd::NDQOpCX::getRhs() const {
    return mChild[I_QARGS]->getChild(I_RHS);
}

void efd::NDQOpCX::setRhs(Node::uRef ref) {
    mChild[I_QARGS]->setChild(I_RHS, std::move(ref));
}

void efd::NDQOpCX::apply(NodeVisitor::Ref visitor) {
    visitor->visit(this);
}

efd::Node::uRef efd::NDQOpCX::cloneImpl() const {
    return NDQOpCX::Create(getLhs()->clone(), getRhs()->clone());
}

bool efd::NDQOpCX::ClassOf(const Node* node) {
    return node->getKind() == K_QOP_CX;
}

efd::NDQOpCX::uRef efd::NDQOpCX::Create(Node::uRef lhsNode, Node::uRef rhsNode) {
    return uRef(new NDQOpCX(std::move(lhsNode), std::move(rhsNode)));
}

// -------------- Qubit Operation: Generic -----------------
efd::NDQOpGen::NDQOpGen(NDId::uRef idNode, NDList::uRef aNode, NDList::uRef qaNode) :
    NDQOp(K_QOP_GEN, std::move(idNode), std::move(aNode), std::move(qaNode)),
    mIK(static_cast<IntrinsicKind>(0)), mIsIntrinsic(false) {}

efd::NDQOpGen::NDQOpGen(NDId::uRef idNode, NDList::uRef aNode, NDList::uRef qaNode,
        IntrinsicKind ik) :
    NDQOp(K_QOP_GEN, std::move(idNode), std::move(aNode), std::move(qaNode)),
    mIK(ik), mIsIntrinsic(true) {}

void efd::NDQOpGen::apply(NodeVisitor::Ref visitor) {
    visitor->visit(this);
}

efd::NDQOpGen::IntrinsicKind efd::NDQOpGen::getIntrinsicKind() const {
    EfdAbortIf(!mIsIntrinsic, "Trying to get IntrinsicKind of non-intrinsic node.");
    return mIK;
}

bool efd::NDQOpGen::isIntrinsic() const {
    return mIsIntrinsic;
}

efd::Node::uRef efd::NDQOpGen::cloneImpl() const {
    auto id = uniqueCastForward<NDId>(getId()->clone());
    auto args = uniqueCastForward<NDList>(getArgs()->clone());
    auto qargs = uniqueCastForward<NDList>(getQArgs()->clone());

    auto cloned = NDQOpGen::Create(std::move(id), std::move(args), std::move(qargs));

    cloned->mIsIntrinsic = mIsIntrinsic;
    cloned->mIK = mIK;
    return Node::uRef(cloned.release());
}

bool efd::NDQOpGen::ClassOf(const Node* node) {
    return node->getKind() == K_QOP_GEN;
}

efd::NDQOpGen::uRef efd::NDQOpGen::Create(NDId::uRef idNode, NDList::uRef aNode, NDList::uRef qaNode) {
    return uRef(new NDQOpGen(std::move(idNode), std::move(aNode), std::move(qaNode)));
}

// -------------- Binary Operation -----------------
efd::NDBinOp::NDBinOp(OpType t, Node::uRef lhsNode, Node::uRef rhsNode) :
    Node(K_BINOP), mT(t) {
    innerAddChild(std::move(lhsNode));
    innerAddChild(std::move(rhsNode));
}

efd::Node::uRef efd::NDBinOp::cloneImpl() const {
    return NDBinOp::Create(mT, getLhs()->clone(), getRhs()->clone());
}

efd::Node::Ref efd::NDBinOp::getLhs() const {
    return mChild[I_LHS].get();
}

void efd::NDBinOp::setLhs(Node::uRef ref) {
    setChild(I_LHS, std::move(ref));
}

efd::Node::Ref efd::NDBinOp::getRhs() const {
    return mChild[I_RHS].get();
}

void efd::NDBinOp::setRhs(Node::uRef ref) {
    setChild(I_RHS, std::move(ref));
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
        default: EfdAbortIf(true, "Unknown binary operation.");
    }
}

uint32_t efd::NDBinOp::getChildNumber() const {
    return 2;
}

void efd::NDBinOp::apply(NodeVisitor::Ref visitor) {
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
efd::NDUnaryOp::NDUnaryOp(UOpType t, Node::uRef oNode) : Node(K_UNARYOP), mT(t) {
    innerAddChild(std::move(oNode));
}

efd::Node::uRef efd::NDUnaryOp::cloneImpl() const {
    return NDUnaryOp::Create(mT, getOperand()->clone());
}

efd::Node::Ref efd::NDUnaryOp::getOperand() const {
    return mChild[I_ONLY].get();
}

void efd::NDUnaryOp::setOperand(Node::uRef ref) {
    setChild(I_ONLY, std::move(ref));
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
        default: EfdAbortIf(true, "Unknown unary operation.");
    }
}

uint32_t efd::NDUnaryOp::getChildNumber() const {
    return 1;
}

void efd::NDUnaryOp::apply(NodeVisitor::Ref visitor) {
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
