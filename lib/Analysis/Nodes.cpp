#include "enfield/Analysis/Nodes.h"
#include "enfield/Analysis/NodeVisitor.h"
#include "enfield/Support/RTTI.h"

efd::Node::Node(Kind k, unsigned size, bool empty) : mK(k), mIsEmpty(empty),
    mChild(size, nullptr) {
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

std::string efd::Node::getOperation() const {
    return "";
}

unsigned efd::Node::getChildNumber() const {
    return 0;
}

// -------------- QasmVersion -----------------
efd::NDQasmVersion::NDQasmVersion(NodeRef vNode, NodeRef stmtsNode) :
    Node(K_QASM_VERSION, getChildNumber()) {
    mChild[I_VERSION] = vNode;
    mChild[I_STMTS] = stmtsNode;
}

efd::NodeRef efd::NDQasmVersion::clone() const {
    return NDQasmVersion::Create(getVersion()->clone(), getStatements()->clone());
}

std::string efd::NDQasmVersion::getOperation() const {
    return "OPENQASM";
}

unsigned efd::NDQasmVersion::getChildNumber() const {
    return 2;
}

void efd::NDQasmVersion::apply(NodeVisitor* visitor) {
    visitor->visit(this);
}

efd::NDReal* efd::NDQasmVersion::getVersion() const {
    return dynCast<NDReal>(mChild[I_VERSION]);
}

efd::NDStmtList* efd::NDQasmVersion::getStatements() const {
    return dynCast<NDStmtList>(mChild[I_STMTS]);
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

bool efd::NDQasmVersion::ClassOf(const NodeRef node) {
    return node->getKind() == K_DECL;
}

efd::NodeRef efd::NDQasmVersion::Create(NodeRef vNode, NodeRef stmtsNode) {
    return new NDQasmVersion(vNode, stmtsNode);
}

// -------------- Include -----------------
efd::NDInclude::NDInclude(NodeRef vNode, NodeRef stmtsNode) :
    Node(K_QASM_VERSION, getChildNumber()) {
    mChild[I_FILE] = vNode;
    mChild[I_STMTS] = stmtsNode;
}

efd::NodeRef efd::NDInclude::clone() const {
    return NDInclude::Create(getFilename()->clone(), getStatements()->clone());
}

std::string efd::NDInclude::getOperation() const {
    return "include";
}

unsigned efd::NDInclude::getChildNumber() const {
    return 2;
}

void efd::NDInclude::apply(NodeVisitor* visitor) {
    visitor->visit(this);
}

efd::NDString* efd::NDInclude::getFilename() const {
    return dynCast<NDString>(mChild[I_FILE]);
}

efd::NDStmtList* efd::NDInclude::getStatements() const {
    return dynCast<NDStmtList>(mChild[I_STMTS]);
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
    return K_QASM_VERSION;
}

bool efd::NDInclude::ClassOf(const NodeRef node) {
    return node->getKind() == K_DECL;
}

efd::NodeRef efd::NDInclude::Create(NodeRef vNode, NodeRef stmtsNode) {
    return new NDInclude(vNode, stmtsNode);
}

// -------------- Decl -----------------
efd::NDDecl::NDDecl(Type t, NodeRef idNode, NodeRef sizeNode) : Node(K_DECL, getChildNumber()), mT(t) {
    mChild[I_ID] = idNode;
    mChild[I_SIZE] = sizeNode;
}

efd::NodeRef efd::NDDecl::clone() const {
    return NDDecl::Create(mT, getId()->clone(), getSize()->clone());
}

efd::NDId* efd::NDDecl::getId() const {
    return dynCast<NDId>(mChild[I_ID]);
}

efd::NDInt* efd::NDDecl::getSize() const {
    return dynCast<NDInt>(mChild[I_SIZE]);
}

bool efd::NDDecl::isCReg() const {
    return mT == CONCRETE;
}

bool efd::NDDecl::isQReg() const {
    return mT == QUANTUM;
}

std::string efd::NDDecl::getOperation() const {
    switch (mT) {
        case CONCRETE: return "creg";
        case QUANTUM:  return "qreg";
    }
}

unsigned efd::NDDecl::getChildNumber() const {
    return 2;
}

void efd::NDDecl::apply(NodeVisitor* visitor) {
    visitor->visit(this);
}

std::string efd::NDDecl::toString(bool pretty) const {
    std::string str;
    std::string endl = (pretty) ? "\n" : "";

    str += getOperation();
    str += " " + getId()->toString(pretty);
    str += "[" + getSize()->toString(pretty) + "];";
    str += endl;

    return str;
}

efd::Node::Kind efd::NDDecl::getKind() const {
    return K_DECL;
}

bool efd::NDDecl::ClassOf(const NodeRef node) {
    return node->getKind() == K_DECL;
}

efd::NodeRef efd::NDDecl::Create(Type t, NodeRef idNode, NodeRef sizeNode) {
    return new NDDecl(t, idNode, sizeNode);
}

// -------------- GateDecl -----------------
efd::NDGateDecl::NDGateDecl(NodeRef idNode, NodeRef aNode, NodeRef qaNode, NodeRef gopNode) : Node(K_GATE_DECL, getChildNumber()) {
    mChild[I_ID] = idNode;
    mChild[I_ARGS] = aNode;
    mChild[I_QARGS] = qaNode;
    mChild[I_GOPLIST] = gopNode;
}

efd::NodeRef efd::NDGateDecl::clone() const {
    return NDGateDecl::Create(
            getId()->clone(), getArgs()->clone(), 
            getQArgs()->clone(), getGOpList()->clone()
        );
}

efd::NDId* efd::NDGateDecl::getId() const {
    return dynCast<NDId>(mChild[I_ID]);
}

efd::NDList* efd::NDGateDecl::getArgs() const {
    return dynCast<NDList>(mChild[I_ARGS]);
}

efd::NDList* efd::NDGateDecl::getQArgs() const {
    return dynCast<NDList>(mChild[I_QARGS]);
}

efd::NDGOpList* efd::NDGateDecl::getGOpList() const {
    return dynCast<NDGOpList>(mChild[I_GOPLIST]);
}

std::string efd::NDGateDecl::getOperation() const {
    return "gate";
}

unsigned efd::NDGateDecl::getChildNumber() const {
    return 4;
}

void efd::NDGateDecl::apply(NodeVisitor* visitor) {
    visitor->visit(this);
}

std::string efd::NDGateDecl::toString(bool pretty) const {
    std::string str;
    std::string endl = (pretty) ? "\n" : "";

    str += getOperation();
    str += " " + getId()->toString(pretty);

    NodeRef refArgs = getArgs();
    if (!refArgs->isEmpty())
        str += "(" + refArgs->toString(pretty) + ")";

    str += " " + getQArgs()->toString(pretty) + " {";
    str += endl;

    NodeRef refGOpList = getGOpList();
    if (!refGOpList->isEmpty())
        str += refGOpList->toString(pretty);
    str += "}";
    str += endl;

    return str;
}

efd::Node::Kind efd::NDGateDecl::getKind() const {
    return K_GATE_DECL;
}

bool efd::NDGateDecl::ClassOf(const NodeRef node) {
    return node->getKind() == K_GATE_DECL;
}

efd::NodeRef efd::NDGateDecl::Create(NodeRef idNode, NodeRef aNode, NodeRef qaNode, NodeRef gopNode) {
    return new NDGateDecl(idNode, aNode, qaNode, gopNode);
}

// -------------- Opaque -----------------
efd::NDOpaque::NDOpaque(NodeRef idNode, NodeRef aNode, NodeRef qaNode) : Node(K_GATE_OPAQUE, getChildNumber()) {
    mChild[I_ID] = idNode;
    mChild[I_ARGS] = aNode;
    mChild[I_QARGS] = qaNode;
}

efd::NodeRef efd::NDOpaque::clone() const {
    return NDOpaque::Create(getId()->clone(), getArgs()->clone(), getQArgs()->clone());
}

efd::NDId* efd::NDOpaque::getId() const {
    return dynCast<NDId>(mChild[I_ID]);
}

efd::NDList* efd::NDOpaque::getArgs() const {
    return dynCast<NDList>(mChild[I_ARGS]);
}

efd::NDList* efd::NDOpaque::getQArgs() const {
    return dynCast<NDList>(mChild[I_QARGS]);
}

std::string efd::NDOpaque::getOperation() const {
    return "opaque";
}

unsigned efd::NDOpaque::getChildNumber() const {
    return 3;
}

void efd::NDOpaque::apply(NodeVisitor* visitor) {
    visitor->visit(this);
}

std::string efd::NDOpaque::toString(bool pretty) const {
    std::string str;
    std::string endl = (pretty) ? "\n" : "";

    str += getOperation();
    str += " " + getId()->toString(pretty);

    NodeRef refArgs = getArgs();
    if (!refArgs->isEmpty())
        str += "(" + refArgs->toString(pretty) + ")";

    str += " " + getQArgs()->toString(pretty) + ";";
    str += endl;

    return str;
}

efd::Node::Kind efd::NDOpaque::getKind() const {
    return K_GATE_OPAQUE;
}

bool efd::NDOpaque::ClassOf(const NodeRef node) {
    return node->getKind() == K_GATE_OPAQUE;
}

efd::NodeRef efd::NDOpaque::Create(NodeRef idNode, NodeRef aNode, NodeRef qaNode) {
    return new NDOpaque(idNode, aNode, qaNode);
}

// -------------- Qubit Operation -----------------
efd::NDQOp::NDQOp(Kind k, unsigned size) : Node(k, size) {
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

bool efd::NDQOp::ClassOf(const NodeRef node) {
    return node->getKind() == K_QOP_MEASURE ||
        node->getKind() == K_QOP_RESET ||
        node->getKind() == K_QOP_BARRIER ||
        node->getKind() == K_QOP_CX ||
        node->getKind() == K_QOP_U ||
        node->getKind() == K_QOP_GENERIC;
}

// -------------- Qubit Operation: Measure -----------------
efd::NDQOpMeasure::NDQOpMeasure(NodeRef qNode, NodeRef cNode) : NDQOp(K_QOP_MEASURE, getChildNumber()) {
    mChild[I_QBIT] = qNode;
    mChild[I_CBIT] = cNode;
}

efd::NodeRef efd::NDQOpMeasure::clone() const {
    return NDQOpMeasure::Create(getQBit()->clone(), getCBit()->clone());
}

efd::NodeRef efd::NDQOpMeasure::getQBit() const {
    return mChild[I_QBIT];
}

efd::NodeRef efd::NDQOpMeasure::getCBit() const {
    return mChild[I_CBIT];
}

std::string efd::NDQOpMeasure::getOperation() const {
    return "measure";
}

unsigned efd::NDQOpMeasure::getChildNumber() const {
    return 2;
}

void efd::NDQOpMeasure::apply(NodeVisitor* visitor) {
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

bool efd::NDQOpMeasure::ClassOf(const NodeRef node) {
    return node->getKind() == K_QOP_MEASURE;
}

efd::NodeRef efd::NDQOpMeasure::Create(NodeRef qNode, NodeRef cNode) {
    return new NDQOpMeasure(qNode, cNode);
}

// -------------- Qubit Operation: Reset -----------------
efd::NDQOpReset::NDQOpReset(NodeRef qaNode) : NDQOp(K_QOP_RESET, getChildNumber()) {
    mChild[I_ONLY] = qaNode;
}

efd::NodeRef efd::NDQOpReset::clone() const {
    return NDQOpReset::Create(getQArg()->clone());
}

efd::NodeRef efd::NDQOpReset::getQArg() const {
    return mChild[I_ONLY];
}

std::string efd::NDQOpReset::getOperation() const {
    return "reset";
}

unsigned efd::NDQOpReset::getChildNumber() const {
    return 1;
}

void efd::NDQOpReset::apply(NodeVisitor* visitor) {
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

bool efd::NDQOpReset::ClassOf(const NodeRef node) {
    return node->getKind() == K_QOP_RESET;
}

efd::NodeRef efd::NDQOpReset::Create(NodeRef qaNode) {
    return new NDQOpReset(qaNode);
}

// -------------- Qubit Operation: Barrier -----------------
efd::NDQOpBarrier::NDQOpBarrier(NodeRef qaNode) : NDQOp(K_QOP_BARRIER, getChildNumber()) {
    mChild[I_ONLY] = qaNode;
}

efd::NodeRef efd::NDQOpBarrier::clone() const {
    return NDQOpBarrier::Create(getQArgs()->clone());
}

efd::NDList* efd::NDQOpBarrier::getQArgs() const {
    return dynCast<NDList>(mChild[I_ONLY]);
}

std::string efd::NDQOpBarrier::getOperation() const {
    return "barrier";
}

unsigned efd::NDQOpBarrier::getChildNumber() const {
    return 1;
}

void efd::NDQOpBarrier::apply(NodeVisitor* visitor) {
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

bool efd::NDQOpBarrier::ClassOf(const NodeRef node) {
    return node->getKind() == K_QOP_BARRIER;
}

efd::NodeRef efd::NDQOpBarrier::Create(NodeRef qaNode) {
    return new NDQOpBarrier(qaNode);
}

// -------------- Qubit Operation: CX -----------------
efd::NDQOpCX::NDQOpCX(NodeRef lhsNode, NodeRef rhsNode) : NDQOp(K_QOP_CX, getChildNumber()) {
    mChild[I_LHS] = lhsNode;
    mChild[I_RHS] = rhsNode;
}

efd::NodeRef efd::NDQOpCX::clone() const {
    return NDQOpCX::Create(getLhs()->clone(), getRhs()->clone());
}

efd::NodeRef efd::NDQOpCX::getLhs() const {
    return mChild[I_LHS];
}

efd::NodeRef efd::NDQOpCX::getRhs() const {
    return mChild[I_RHS];
}

std::string efd::NDQOpCX::getOperation() const {
    return "CX";
}

unsigned efd::NDQOpCX::getChildNumber() const {
    return 2;
}

void efd::NDQOpCX::apply(NodeVisitor* visitor) {
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

bool efd::NDQOpCX::ClassOf(const NodeRef node) {
    return node->getKind() == K_QOP_CX;
}

efd::NodeRef efd::NDQOpCX::Create(NodeRef lhsNode, NodeRef rhsNode) {
    return new NDQOpCX(lhsNode, rhsNode);
}

// -------------- Qubit Operation: U -----------------
efd::NDQOpU::NDQOpU(NodeRef aNode, NodeRef qaNode) : NDQOp(K_QOP_U, getChildNumber()) {
    mChild[I_ARGS] = aNode;
    mChild[I_QARG] = qaNode;
}

efd::NodeRef efd::NDQOpU::clone() const {
    return NDQOpU::Create(getArgs()->clone(), getQArg()->clone());
}

efd::NodeRef efd::NDQOpU::getArgs() const {
    return mChild[I_ARGS];
}

efd::NodeRef efd::NDQOpU::getQArg() const {
    return mChild[I_QARG];
}

std::string efd::NDQOpU::getOperation() const {
    return "U";
}

unsigned efd::NDQOpU::getChildNumber() const {
    return 2;
}

void efd::NDQOpU::apply(NodeVisitor* visitor) {
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

bool efd::NDQOpU::ClassOf(const NodeRef node) {
    return node->getKind() == K_QOP_U;
}

efd::NodeRef efd::NDQOpU::Create(NodeRef aNode, NodeRef qaNode) {
    return new NDQOpU(aNode, qaNode);
}

// -------------- Qubit Operation: Generic -----------------
efd::NDQOpGeneric::NDQOpGeneric(NodeRef idNode, NodeRef aNode, NodeRef qaNode) : 
    NDQOp(K_QOP_GENERIC, getChildNumber()) {
    mChild[I_ID] = idNode;
    mChild[I_ARGS] = aNode;
    mChild[I_QARGS] = qaNode;
}

efd::NodeRef efd::NDQOpGeneric::clone() const {
    return NDQOpGeneric::Create(getId()->clone(), getArgs()->clone(), getQArgs()->clone());
}

efd::NDId* efd::NDQOpGeneric::getId() const {
    return dynCast<NDId>(mChild[I_ID]);
}

efd::NDList* efd::NDQOpGeneric::getArgs() const {
    return dynCast<NDList>(mChild[I_ARGS]);
}

efd::NDList* efd::NDQOpGeneric::getQArgs() const {
    return dynCast<NDList>(mChild[I_QARGS]);
}

std::string efd::NDQOpGeneric::getOperation() const {
    return getId()->toString();
}

unsigned efd::NDQOpGeneric::getChildNumber() const {
    return 3;
}

void efd::NDQOpGeneric::apply(NodeVisitor* visitor) {
    visitor->visit(this);
}

std::string efd::NDQOpGeneric::toString(bool pretty) const {
    std::string str;
    std::string endl = (pretty) ? "\n" : "";

    str += getOperation();

    NodeRef refArgs = getArgs();
    if (!refArgs->isEmpty())
        str += "(" + refArgs->toString(pretty) + ")";

    str += " " + getQArgs()->toString(pretty) + ";";
    str += endl;

    return str;
}

efd::Node::Kind efd::NDQOpGeneric::getKind() const {
    return K_QOP_GENERIC;
}

bool efd::NDQOpGeneric::ClassOf(const NodeRef node) {
    return node->getKind() == K_QOP_GENERIC;
}

efd::NodeRef efd::NDQOpGeneric::Create(NodeRef idNode, NodeRef aNode, NodeRef qaNode) {
    return new NDQOpGeneric(idNode, aNode, qaNode);
}

// -------------- Binary Operation -----------------
efd::NDBinOp::NDBinOp(OpType t, NodeRef lhsNode, NodeRef rhsNode) : Node(K_BINOP, getChildNumber()), mT(t) {
    mChild[I_LHS] = lhsNode;
    mChild[I_RHS] = rhsNode;
}

efd::NodeRef efd::NDBinOp::clone() const {
    return NDBinOp::Create(mT, getLhs()->clone(), getRhs()->clone());
}

efd::NodeRef efd::NDBinOp::getLhs() const {
    return mChild[I_LHS];
}

efd::NodeRef efd::NDBinOp::getRhs() const {
    return mChild[I_RHS];
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

void efd::NDBinOp::apply(NodeVisitor* visitor) {
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

bool efd::NDBinOp::ClassOf(const NodeRef node) {
    return node->getKind() == K_BINOP;
}

efd::NodeRef efd::NDBinOp::Create(OpType t, NodeRef lhsNode, NodeRef rhsNode) {
    return new NDBinOp(t, lhsNode, rhsNode);
}

// -------------- Unary Operation -----------------
efd::NDUnaryOp::NDUnaryOp(UOpType t, NodeRef oNode) : Node(K_UNARYOP, getChildNumber()), mT(t) {
    mChild[I_ONLY] = oNode;
}

efd::NodeRef efd::NDUnaryOp::clone() const {
    return NDUnaryOp::Create(mT, getOperand()->clone());
}

efd::NodeRef efd::NDUnaryOp::getOperand() const {
    return mChild[I_ONLY];
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

void efd::NDUnaryOp::apply(NodeVisitor* visitor) {
    visitor->visit(this);
}

std::string efd::NDUnaryOp::toString(bool pretty) const {
    std::string str;

    NodeRef refOperand = getOperand();
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

bool efd::NDUnaryOp::ClassOf(const NodeRef node) {
    return node->getKind() == K_UNARYOP;
}

efd::NodeRef efd::NDUnaryOp::Create(UOpType t, NodeRef oNode) {
    return new NDUnaryOp(t, oNode);
}

// -------------- ID reference Operation -----------------
efd::NDIdRef::NDIdRef(NodeRef idNode, NodeRef nNode) : Node(K_ID_REF, getChildNumber()) {
    mChild[I_ID] = idNode;
    mChild[I_N] = nNode;
}

efd::NodeRef efd::NDIdRef::clone() const {
    return NDIdRef::Create(getId()->clone(), getN()->clone());
}

efd::NDId* efd::NDIdRef::getId() const {
    return dynCast<NDId>(mChild[I_ID]);
}

efd::NDInt* efd::NDIdRef::getN() const {
    return dynCast<NDInt>(mChild[I_N]);
}

unsigned efd::NDIdRef::getChildNumber() const {
    return 2;
}

void efd::NDIdRef::apply(NodeVisitor* visitor) {
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

bool efd::NDIdRef::ClassOf(const NodeRef node) {
    return node->getKind() == K_ID_REF;
}

efd::NodeRef efd::NDIdRef::Create(NodeRef idNode, NodeRef sizeNode) {
    return new NDIdRef(idNode, sizeNode);
}

// -------------- Node List -----------------
efd::NDList::NDList() : Node(K_LIST, 0, true) {
}

efd::NDList::NDList(Kind k, unsigned size) : Node(k, size, true) {
}

void efd::NDList::cloneChildremTo(NDList* list) const {
    for (auto child : *this) {
        list->addChild(child->clone());
    }
}

efd::NodeRef efd::NDList::clone() const {
    NDList* list = dynCast<NDList>(NDList::Create());
    cloneChildremTo(list);
    return list;
}

efd::NodeRef efd::NDList::getChild(unsigned i) const {
    return mChild[i];
}

void efd::NDList::addChild(NodeRef child) {
    mChild.push_back(child);
    Node::mIsEmpty = false;
}

unsigned efd::NDList::getChildNumber() const {
    return mChild.size();
}

void efd::NDList::apply(NodeVisitor* visitor) {
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

bool efd::NDList::ClassOf(const NodeRef node) {
    return node->getKind() == K_LIST ||
        node->getKind() == K_STMT_LIST ||
        node->getKind() == K_GOP_LIST;
}

efd::NodeRef efd::NDList::Create() {
    return new NDList();
}

// -------------- StmtList -----------------
efd::NDStmtList::NDStmtList() : NDList(K_STMT_LIST, 0) {
}

efd::NodeRef efd::NDStmtList::clone() const {
    NDList* list = dynCast<NDList>(NDStmtList::Create());
    cloneChildremTo(list);
    return list;
}

void efd::NDStmtList::apply(NodeVisitor* visitor) {
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

bool efd::NDStmtList::ClassOf(const NodeRef node) {
    return node->getKind() == K_STMT_LIST; 
}

efd::NodeRef efd::NDStmtList::Create() {
    return new NDStmtList();
}

// -------------- GOpList -----------------
efd::NDGOpList::NDGOpList() : NDList(K_GOP_LIST, 0) {
}

efd::NodeRef efd::NDGOpList::clone() const {
    NDList* list = dynCast<NDList>(NDGOpList::Create());
    cloneChildremTo(list);
    return list;
}

void efd::NDGOpList::apply(NodeVisitor* visitor) {
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

bool efd::NDGOpList::ClassOf(const NodeRef node) {
    return node->getKind() == K_GOP_LIST; 
}

efd::NodeRef efd::NDGOpList::Create() {
    return new NDGOpList();
}

// -------------- If Statement -----------------
efd::NDIfStmt::NDIfStmt(NodeRef cidNode, NodeRef nNode, NodeRef qopNode) : Node(K_IF_STMT, getChildNumber()) {
    mChild[I_COND_ID] = cidNode;
    mChild[I_COND_N] = nNode;
    mChild[I_QOP] = qopNode;
}

efd::NodeRef efd::NDIfStmt::clone() const {
    return NDIfStmt::Create(getCondId()->clone(), getCondN()->clone(), getQOp()->clone());
}

efd::NDId* efd::NDIfStmt::getCondId() const {
    return dynCast<NDId>(mChild[I_COND_ID]);
}

efd::NDInt* efd::NDIfStmt::getCondN() const {
    return dynCast<NDInt>(mChild[I_COND_N]);
}

efd::NodeRef efd::NDIfStmt::getQOp() const {
    return mChild[I_QOP];
}

unsigned efd::NDIfStmt::getChildNumber() const {
    return 3;
}

void efd::NDIfStmt::apply(NodeVisitor* visitor) {
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

bool efd::NDIfStmt::ClassOf(const NodeRef node) {
    return node->getKind() == K_IF_STMT; 
}

efd::NodeRef efd::NDIfStmt::Create(NodeRef cidNode, NodeRef intNode, NodeRef qopNode) {
    return new NDIfStmt(cidNode, intNode, qopNode);
}

// -------------- Value Specializations -----------------
// -------------- Value<efd::IntVal> -----------------
template <> 
efd::NDValue<efd::IntVal>::NDValue(efd::IntVal val) 
    : Node(K_LIT_INT, getChildNumber()), mVal(val) {
}

template <> 
bool efd::NDValue<efd::IntVal>::ClassOf(const NodeRef node) { 
    return node->getKind() == K_LIT_INT; 
}

template <> 
efd::Node::Kind efd::NDValue<efd::IntVal>::getKind() const {
    return K_LIT_INT; 
}

template <> 
void efd::NDValue<efd::IntVal>::apply(NodeVisitor* visitor) {
    visitor->visit(this);
}

// -------------- Value<efd::RealVal> -----------------
template <> 
efd::NDValue<efd::RealVal>::NDValue(efd::RealVal val) : 
    Node(K_LIT_REAL), mVal(val) {
}

template <> 
bool efd::NDValue<efd::RealVal>::ClassOf(const NodeRef node) {
    return node->getKind() == K_LIT_REAL; 
}

template <> 
efd::Node::Kind efd::NDValue<efd::RealVal>::getKind() const { 
    return K_LIT_REAL; 
}

template <> 
void efd::NDValue<efd::RealVal>::apply(NodeVisitor* visitor) {
    visitor->visit(this);
}

// -------------- Value<std::string> -----------------
template <> 
efd::NDValue<std::string>::NDValue(std::string val) 
    : Node(K_LIT_STRING, getChildNumber()), mVal(val) {
}

template <> 
bool efd::NDValue<std::string>::ClassOf(const NodeRef node) {
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
void efd::NDValue<std::string>::apply(NodeVisitor* visitor) {
    visitor->visit(this);
}

template <> 
std::string efd::NDValue<std::string>::toString(bool pretty) const {
    return mVal; 
}
