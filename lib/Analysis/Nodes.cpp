#include "enfield/Analysis/Nodes.h"

efd::Node::Node(Kind k, bool empty) : mK(k), mIsEmpty(empty) {
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

// -------------- Decl -----------------
efd::NDDecl::NDDecl(Type t, NodeRef idNode, NodeRef sizeNode) : Node(K_DECL), mT(t) {
    mChild.push_back(idNode);
    mChild.push_back(sizeNode);
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

std::string efd::NDDecl::toString(bool pretty) const {
    std::string str(getOperation());
    std::string endl = (pretty) ? "\n" : "";

    str += " " + mChild[I_ID]->toString(pretty);
    str += "[" + mChild[I_SIZE]->toString(pretty) + "];";
    str += endl;

    return str;
}

efd::Node::Kind efd::NDDecl::getKind() const {
    return K_DECL;
}

efd::Node::Kind efd::NDDecl::GetKind() {
    return K_DECL;
}

efd::NodeRef efd::NDDecl::create(Type t, NodeRef idNode, NodeRef sizeNode) {
    return new NDDecl(t, idNode, sizeNode);
}

// -------------- GateDecl -----------------
efd::NDGateDecl::NDGateDecl(NodeRef idNode, NodeRef aNode, NodeRef qaNode, NodeRef gopNode) : Node(K_GATE_DECL) {
    mChild.push_back(idNode);
    mChild.push_back(aNode);
    mChild.push_back(qaNode);
    mChild.push_back(gopNode);
}

std::string efd::NDGateDecl::getOperation() const {
    return "gate";
}

std::string efd::NDGateDecl::toString(bool pretty) const {
    std::string str(getOperation());
    std::string endl = (pretty) ? "\n" : "";

    std::string s;

    str += " " + mChild[I_ID]->toString(pretty);

    if (!mChild[I_ARGS]->isEmpty())
        str += "(" + mChild[I_ARGS]->toString(pretty) + ")";

    str += " " + mChild[I_QARGS]->toString(pretty) + " {";
    str += endl;

    if (!mChild[I_GOPLIST]->isEmpty())
        str += mChild[I_GOPLIST]->toString(pretty);
    str += "}";
    str += endl;

    return str;
}

efd::Node::Kind efd::NDGateDecl::getKind() const {
    return K_GATE_DECL;
}

efd::Node::Kind efd::NDGateDecl::GetKind() {
    return K_GATE_DECL;
}

efd::NodeRef efd::NDGateDecl::create(NodeRef idNode, NodeRef aNode, NodeRef qaNode, NodeRef gopNode) {
    return new NDGateDecl(idNode, aNode, qaNode, gopNode);
}

// -------------- Opaque -----------------
efd::NDOpaque::NDOpaque(NodeRef idNode, NodeRef aNode, NodeRef qaNode) : Node(K_GATE_OPAQUE) {
    mChild.push_back(idNode);
    mChild.push_back(aNode);
    mChild.push_back(qaNode);
}

std::string efd::NDOpaque::getOperation() const {
    return "opaque";
}

std::string efd::NDOpaque::toString(bool pretty) const {
    std::string str(getOperation());
    std::string endl = (pretty) ? "\n" : "";

    str += " " + mChild[I_ID]->toString(pretty);

    if (!mChild[I_ARGS]->isEmpty())
        str += "(" + mChild[I_ARGS]->toString(pretty) + ")";

    str += " " + mChild[I_QARGS]->toString(pretty) + ";";
    str += endl;

    return str;
}

efd::Node::Kind efd::NDOpaque::getKind() const {
    return K_GATE_OPAQUE;
}

efd::Node::Kind efd::NDOpaque::GetKind() {
    return K_GATE_OPAQUE;
}

efd::NodeRef efd::NDOpaque::create(NodeRef idNode, NodeRef aNode, NodeRef qaNode) {
    return new NDOpaque(idNode, aNode, qaNode);
}

// -------------- Qubit Operation -----------------
efd::NDQOp::NDQOp(Kind k, QOpType type) : Node(k), mT(type) {
}

efd::NDQOp::QOpType efd::NDQOp::getQOpType() const {
    return mT;
}

bool efd::NDQOp::isReset() const {
    return false;
}

bool efd::NDQOp::isBarrier() const {
    return false;
}

bool efd::NDQOp::isMeasure() const {
    return false;
}

bool efd::NDQOp::isU() const {
    return false;
}

bool efd::NDQOp::isGeneric() const {
    return false;
}

// -------------- Qubit Operation: Measure -----------------
efd::NDQOpMeasure::NDQOpMeasure(NodeRef qNode, NodeRef cNode) : NDQOp(K_QOP_MEASURE, QOP_MEASURE) {
    mChild.push_back(qNode);
    mChild.push_back(cNode);
}

std::string efd::NDQOpMeasure::getOperation() const {
    return "measure";
}

std::string efd::NDQOpMeasure::toString(bool pretty) const {
    std::string str(getOperation());
    std::string endl = (pretty) ? "\n" : "";

    str += " " + mChild[I_QBIT]->toString(pretty);
    str += " ->";
    str += " " + mChild[I_CBIT]->toString(pretty) + ";";

    str += endl;

    return str;
}

efd::Node::Kind efd::NDQOpMeasure::getKind() const {
    return K_QOP_MEASURE;
}

efd::Node::Kind efd::NDQOpMeasure::GetKind() {
    return K_QOP_MEASURE;
}

efd::NodeRef efd::NDQOpMeasure::create(NodeRef qNode, NodeRef cNode) {
    return new NDQOpMeasure(qNode, cNode);
}

// -------------- Qubit Operation: Reset -----------------
efd::NDQOpReset::NDQOpReset(NodeRef qaNode) : NDQOp(K_QOP_RESET, QOP_RESET) {
    mChild.push_back(qaNode);
}

std::string efd::NDQOpReset::getOperation() const {
    return "reset";
}

std::string efd::NDQOpReset::toString(bool pretty) const {
    std::string str;
    std::string endl = (pretty) ? "\n" : "";

    str += getOperation();
    str += " " + mChild[I_ONLY]->toString(pretty) + ";";

    str += endl;

    return str;
}

efd::Node::Kind efd::NDQOpReset::getKind() const {
    return K_QOP_RESET;
}

efd::Node::Kind efd::NDQOpReset::GetKind() {
    return K_QOP_RESET;
}

efd::NodeRef efd::NDQOpReset::create(NodeRef qaNode) {
    return new NDQOpReset(qaNode);
}

// -------------- Qubit Operation: Barrier -----------------
efd::NDQOpBarrier::NDQOpBarrier(NodeRef qaNode) : NDQOp(K_QOP_BARRIER, QOP_BARRIER) {
    mChild.push_back(qaNode);
}

std::string efd::NDQOpBarrier::getOperation() const {
    return "barrier";
}

std::string efd::NDQOpBarrier::toString(bool pretty) const {
    std::string str;
    std::string endl = (pretty) ? "\n" : "";

    str += getOperation();
    str += " " + mChild[I_ONLY]->toString(pretty) + ";";

    str += endl;

    return str;
}

efd::Node::Kind efd::NDQOpBarrier::getKind() const {
    return K_QOP_BARRIER;
}

efd::Node::Kind efd::NDQOpBarrier::GetKind() {
    return K_QOP_BARRIER;
}

efd::NodeRef efd::NDQOpBarrier::create(NodeRef qaNode) {
    return new NDQOpBarrier(qaNode);
}

// -------------- Qubit Operation: Generic -----------------
efd::NDQOpGeneric::NDQOpGeneric(NodeRef idNode, NodeRef aNode, NodeRef qaNode) : NDQOp(K_QOP_GENERIC, QOP_GENERIC) {
    mChild.push_back(idNode);
    mChild.push_back(aNode);
    mChild.push_back(qaNode);
}

std::string efd::NDQOpGeneric::getOperation() const {
    return mChild[I_ID]->toString();
}

std::string efd::NDQOpGeneric::toString(bool pretty) const {
    std::string str;
    std::string endl = (pretty) ? "\n" : "";

    str += getOperation();

    if (!mChild[I_ARGS]->isEmpty())
        str += "(" + mChild[I_ARGS]->toString(pretty) + ")";

    str += " " + mChild[I_QARGS]->toString(pretty) + ";";
    str += endl;

    return str;
}

efd::Node::Kind efd::NDQOpGeneric::getKind() const {
    return K_QOP_GENERIC;
}

efd::Node::Kind efd::NDQOpGeneric::GetKind() {
    return K_QOP_GENERIC;
}

efd::NodeRef efd::NDQOpGeneric::create(NodeRef idNode, NodeRef aNode, NodeRef qaNode) {
    return new NDQOpGeneric(idNode, aNode, qaNode);
}

// -------------- Binary Operation -----------------
efd::NDBinOp::NDBinOp(OpType t, NodeRef lhsNode, NodeRef rhsNode) : Node(K_BINOP), mT(t) {
    mChild.push_back(lhsNode);
    mChild.push_back(rhsNode);
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

std::string efd::NDBinOp::toString(bool pretty) const {
    std::string str;

    str += "(";
    str += mChild[I_LHS]->toString(pretty);
    str += " " + getOperation() + " ";
    str += mChild[I_RHS]->toString(pretty);
    str += ")";

    return str;
}

efd::Node::Kind efd::NDBinOp::getKind() const {
    return K_BINOP;
}

efd::Node::Kind efd::NDBinOp::GetKind() {
    return K_BINOP;
}

efd::NodeRef efd::NDBinOp::create(OpType t, NodeRef lhsNode, NodeRef rhsNode) {
    return new NDBinOp(t, lhsNode, rhsNode);
}

// -------------- Unary Operation -----------------
efd::NDUnaryOp::NDUnaryOp(UOpType t, NodeRef oNode) : Node(K_UNARYOP), mT(t) {
    mChild.push_back(oNode);
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

std::string efd::NDUnaryOp::toString(bool pretty) const {
    std::string str;

    if (mT == UOP_NEG) {
        str += "(";
        str += getOperation();
        str += mChild[I_ONLY]->toString(pretty);
        str += ")";
    } else {
        str += getOperation();
        str += "(" + mChild[I_ONLY]->toString(pretty) + ")";
    }

    return str;
}

efd::Node::Kind efd::NDUnaryOp::getKind() const {
    return K_UNARYOP;
}

efd::Node::Kind efd::NDUnaryOp::GetKind() {
    return K_UNARYOP;
}

efd::NodeRef efd::NDUnaryOp::create(UOpType t, NodeRef oNode) {
    return new NDUnaryOp(t, oNode);
}

// -------------- ID reference Operation -----------------
efd::NDIdRef::NDIdRef(NodeRef idNode, NodeRef sizeNode) : Node(K_ID_REF) {
    mChild.push_back(idNode);
    mChild.push_back(sizeNode);
}

std::string efd::NDIdRef::toString(bool pretty) const {
    std::string str;

    str += mChild[I_ID]->toString(pretty);
    str += "[" + mChild[I_N]->toString(pretty) + "]";

    return str;
}

efd::Node::Kind efd::NDIdRef::getKind() const {
    return K_ID_REF;
}

efd::Node::Kind efd::NDIdRef::GetKind() {
    return K_ID_REF;
}

efd::NodeRef efd::NDIdRef::create(NodeRef idNode, NodeRef sizeNode) {
    return new NDIdRef(idNode, sizeNode);
}

// -------------- Node List -----------------
efd::NDList::NDList() : Node(K_LIST, true) {
}

efd::NDList::NDList(Kind k) : Node(k, true) {
}

void efd::NDList::addChild(NodeRef child) {
    mChild.push_back(child);
    Node::mIsEmpty = false;
}

std::string efd::NDList::toString(bool pretty) const {
    std::string str;

    if (!mChild.empty()) {
        str += mChild[0]->toString(pretty);

        for (auto it = mChild.begin() + 1, end = mChild.end(); it != end; ++it)
            str += ", " + (*it)->toString(pretty);
    }

    return str;
}

efd::Node::Kind efd::NDList::getKind() const {
    return K_LIST;
}

efd::Node::Kind efd::NDList::GetKind() {
    return K_LIST;
}

efd::NodeRef efd::NDList::create() {
    return new NDList();
}

// -------------- StmtList -----------------
efd::NDStmtList::NDStmtList() : NDList(K_STMT_LIST) {
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

efd::Node::Kind efd::NDStmtList::GetKind() {
    return K_STMT_LIST;
}

efd::NodeRef efd::NDStmtList::create() {
    return new NDStmtList();
}

// -------------- GOpList -----------------
efd::NDGOpList::NDGOpList() : NDList(K_STMT_LIST) {
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

efd::Node::Kind efd::NDGOpList::GetKind() {
    return K_GOP_LIST;
}

efd::NodeRef efd::NDGOpList::create() {
    return new NDGOpList();
}

// -------------- Value Specializations -----------------
// -------------- Value<efd::IntVal> -----------------
template <> 
efd::NDValue<efd::IntVal>::NDValue(efd::IntVal val) 
    : Node(K_LIT_INT), mVal(val) {
}

template <> 
efd::Node::Kind efd::NDValue<efd::IntVal>::GetKind() { 
    return K_LIT_INT; 
}

template <> 
efd::Node::Kind efd::NDValue<efd::IntVal>::getKind() const {
    return K_LIT_INT; 
}

// -------------- Value<efd::RealVal> -----------------
template <> 
efd::NDValue<efd::RealVal>::NDValue(efd::RealVal val) : 
    Node(K_LIT_REAL), mVal(val) {
}

template <> 
efd::Node::Kind efd::NDValue<efd::RealVal>::GetKind() {
    return K_LIT_REAL; 
}

template <> 
efd::Node::Kind efd::NDValue<efd::RealVal>::getKind() const { 
    return K_LIT_REAL; 
}

// -------------- Value<std::string> -----------------
template <> 
efd::NDValue<std::string>::NDValue(std::string val) 
    : Node(K_LIT_STRING), mVal(val) {
}

template <> 
efd::Node::Kind efd::NDValue<std::string>::GetKind() {
    return K_LIT_STRING; 
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
std::string efd::NDValue<std::string>::toString(bool pretty) const {
    return mVal; 
}
