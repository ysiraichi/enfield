#include "enfield/Analysis/Nodes.h"

efd::Node::Node(bool empty) : mIsEmpty(empty) {
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

std::string efd::Node::getOperation() const {
    return "";
}

void efd::Node::print(std::ostream& O, bool pretty) {
    O << toString(pretty);
}

void efd::Node::print(bool pretty) {
    std::cout << toString(pretty);
}

// -------------- Decl -----------------
efd::NDDecl::NDDecl(Type t, NodeRef idNode, NodeRef sizeNode) : Node(false), mT(t) {
    mChild.push_back(std::move(idNode));
    mChild.push_back(std::move(sizeNode));
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

    str += " " + mChild[I_ID]->toString(pretty);
    str += "[" + mChild[I_SIZE]->toString(pretty) + "]";

    return str;
}

// -------------- GateDecl -----------------
efd::NDGateDecl::NDGateDecl(NodeRef idNode, NodeRef aNode, NodeRef qaNode, NodeRef gopNode) : Node(false) {
    mChild.push_back(std::move(idNode));
    mChild.push_back(std::move(aNode));
    mChild.push_back(std::move(qaNode));
    mChild.push_back(std::move(gopNode));
}

std::string efd::NDGateDecl::getOperation() const {
    return "gate";
}

std::string efd::NDGateDecl::toString(bool pretty) const {
    std::string str(getOperation());
    std::string endl = (pretty) ? "\n" : "";

    str += " " + mChild[I_ID]->toString(pretty);

    if (!mChild[I_ARGS]->isEmpty())
        str += " (" + mChild[I_ARGS]->toString(pretty) + ")";

    str += " " + mChild[I_QARGS]->toString(pretty) + "{" + endl;
    str += mChild[I_GOPLIST]->toString(pretty) + endl + "}" + endl;

    return str;
}

// -------------- Opaque -----------------
efd::NDOpaque::NDOpaque(NodeRef idNode, NodeRef aNode, NodeRef qaNode) : Node(false) {
    mChild.push_back(std::move(idNode));
    mChild.push_back(std::move(aNode));
    mChild.push_back(std::move(qaNode));
}

std::string efd::NDOpaque::getOperation() const {
    return "opaque";
}

std::string efd::NDOpaque::toString(bool pretty) const {
    std::string str(getOperation());
    std::string endl = (pretty) ? "\n" : "";

    str += " " + mChild[I_ID]->toString(pretty);

    if (!mChild[I_ARGS]->isEmpty())
        str += " (" + mChild[I_ARGS]->toString(pretty) + ")";

    str += " " + mChild[I_QARGS]->toString(pretty) + ";";

    return str;
}

// -------------- Qubit Operation -----------------
efd::NDQOp::NDQOp(QOpType type) : Node(false), mT(type) {
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
efd::NDQOpMeasure::NDQOpMeasure(NodeRef qNode, NodeRef cNode) : NDQOp(QOP_MEASURE) {
    mChild.push_back(std::move(qNode));
    mChild.push_back(std::move(cNode));
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

    return str;
}

// -------------- Qubit Operation: Reset -----------------
efd::NDQOpReset::NDQOpReset(NodeRef qaNode) : NDQOp(QOP_RESET) {
    mChild.push_back(std::move(qaNode));
}

std::string efd::NDQOpReset::getOperation() const {
    return "reset";
}

std::string efd::NDQOpReset::toString(bool pretty) const {
    std::string str;

    str += getOperation();
    str += " " + mChild[I_ONLY]->toString(pretty);

    return str;
}

// -------------- Qubit Operation: Barrier -----------------
efd::NDQOpBarrier::NDQOpBarrier(NodeRef qaNode) : NDQOp(QOP_BARRIER) {
    mChild.push_back(std::move(qaNode));
}

std::string efd::NDQOpBarrier::getOperation() const {
    return "barrier";
}

std::string efd::NDQOpBarrier::toString(bool pretty) const {
    std::string str;

    str += getOperation();
    str += " " + mChild[I_ONLY]->toString(pretty);

    return str;
}

// -------------- Qubit Operation: Generic -----------------
efd::NDQOpGeneric::NDQOpGeneric(NodeRef idNode, NodeRef aNode, NodeRef qaNode) : NDQOp(QOP_GENERIC) {
    mChild.push_back(std::move(idNode));
    mChild.push_back(std::move(aNode));
    mChild.push_back(std::move(qaNode));
}

std::string efd::NDQOpGeneric::getOperation() const {
    return mChild[I_ID]->toString();
}

std::string efd::NDQOpGeneric::toString(bool pretty) const {
    std::string str;

    str += getOperation();

    if (!mChild[I_ARGS]->isEmpty())
        str += "(" + mChild[I_ARGS]->toString(pretty) + ")";

    str += " " + mChild[I_QARGS]->toString(pretty) + ";";

    return str;
}

// -------------- Binary Operation -----------------
efd::NDBinOp::NDBinOp(OpType t, NodeRef lhsNode, NodeRef rhsNode) : Node(false), mT(t) {
    mChild.push_back(std::move(lhsNode));
    mChild.push_back(std::move(rhsNode));
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

// -------------- Unary Operation -----------------
efd::NDUnaryOp::NDUnaryOp(UOpType t, NodeRef oNode) : Node(false), mT(t) {
    mChild.push_back(std::move(oNode));
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
        case UOP_COS:   return "con";
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

// -------------- ID reference Operation -----------------
efd::NDIdRef::NDIdRef(NodeRef idNode, NodeRef sizeNode) : Node(false) {
    mChild.push_back(std::move(idNode));
    mChild.push_back(std::move(sizeNode));
}

std::string efd::NDIdRef::toString(bool pretty) const {
    std::string str;

    str += mChild[I_ID]->toString(pretty);
    str += "[" + mChild[I_N]->toString(pretty) + "]";

    return str;
}

// -------------- Node List -----------------
efd::NDList::NDList() : Node(false) {
}

void efd::NDList::addChild(NodeRef child) {
    mChild.push_back(std::move(child));
}

// -------------- Arg list Operation -----------------
std::string efd::NDArgList::toString(bool pretty) const {
    std::string str;

    if (!mChild.empty()) {
        str += mChild[0]->toString(pretty);

        for (auto &arg : *this)
            str += ", " + arg->toString(pretty);
    }

    return str;
}

// -------------- GOpList -----------------
std::string efd::NDGOpList::toString(bool pretty) const {
    std::string str;
    std::string endl = (pretty) ? "\n" : "";
    std::string tab = (pretty) ? "\t" : "";

    for (auto &child : *this)
        str += tab + child->toString(pretty) + ";" + endl;

    return str;
}
