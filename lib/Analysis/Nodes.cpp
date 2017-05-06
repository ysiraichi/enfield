#include "enfield/Analysis/Nodes.h"

efd::Node::Node(bool empty) : mIsEmpty(empty) {
}

efd::Node::iterator efd::Node::begin() {
    return mChild.begin();
}

efd::Node::iterator efd::Node::end() {
    return mChild.end();
}

efd::Node::const_iterator efd::Node::begin() const {
    return mChild.begin();
}

efd::Node::const_iterator efd::Node::end() const {
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

std::string efd::Node::toString(bool pretty) const {
    std::string str(getOperation());

    for (auto &child : *this)
        str += " " + child->toString(pretty);

    return str;
}

// -------------- Decl -----------------
efd::NDDecl::NDDecl(Type t) : Node(false), mT(t) {
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
efd::NDGateDecl::NDGateDecl() : Node(false) {
}

std::string efd::NDGateDecl::getOperation() const {
    return "gate";
}

std::string efd::NDGateDecl::toString(bool pretty) const {
    std::string str(getOperation());
    std::string endl = (pretty) ? "\n" : "";

    str += " " + mChild[I_ID]->toString(pretty);

    if (!mChild[I_ARGS]->mChild.empty())
        str += " (" + mChild[I_ARGS]->toString(pretty) + ")";

    str += " " + mChild[I_QARGS]->toString(pretty) + "{" + endl;
    str += mChild[I_GOPLIST]->toString(pretty) + endl + "}" + endl;

    return str;
}

// -------------- GOpList -----------------
efd::NDGOpList::NDGOpList() : Node(false) {
}

std::string efd::NDGOpList::toString(bool pretty) const {
    std::string str;
    std::string endl = (pretty) ? "\n" : "";
    std::string tab = (pretty) ? "\t" : "";

    for (auto &child : *this)
        str += tab + child->toString(pretty) + ";" + endl;

    return str;
}

// -------------- Opaque -----------------
efd::NDOpaque::NDOpaque() : Node(false) {
}

std::string efd::NDOpaque::getOperation() const {
    return "opaque";
}

std::string efd::NDOpaque::toString(bool pretty) const {
    std::string str(getOperation());
    std::string endl = (pretty) ? "\n" : "";

    str += " " + mChild[I_ID]->toString(pretty);

    if (!mChild[I_ARGS]->mChild.empty())
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

std::string efd::NDQOp::getOperation() const {
    return mChild[I_ID]->toString();
}

std::string efd::NDQOp::toString(bool pretty) const {
    std::string str(getOperation());
    std::string endl = (pretty) ? "\n" : "";

    if (!mChild[I_ARGS]->isEmpty())
        str += "(" + mChild[I_ARGS]->toString(pretty) + ")";

    str += " " + mChild[I_QARGS]->toString(pretty) + ";";

    return str;
}

// -------------- Qubit Operation: Measure -----------------
efd::NDQOpMeasure::NDQOpMeasure() : NDQOp(QOP_MEASURE) {
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
efd::NDQOpReset::NDQOpReset() : NDQOp(QOP_RESET) {
}

std::string efd::NDQOpReset::getOperation() const {
    return "reset";
}

// -------------- Qubit Operation: Barrier -----------------
efd::NDQOpBarrier::NDQOpBarrier() : NDQOp(QOP_BARRIER) {
}

std::string efd::NDQOpBarrier::getOperation() const {
    return "barrier";
}

// -------------- Qubit Operation: Generic -----------------
efd::NDQOpGeneric::NDQOpGeneric() : NDQOp(QOP_GENERIC) {
}

// -------------- Binary Operation -----------------
efd::NDBinOp::NDBinOp(OpType t) : Node(false), mT(t) {
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

bool efd::NDBinOp::isNeg() const {
    return mT == OP_NEG;
}

std::string efd::NDBinOp::getOperation() const {
    switch (mT) {
        case OP_ADD: return "+";
        case OP_SUB: return "-";
        case OP_MUL: return "*";
        case OP_DIV: return "/";
        case OP_POW: return "^";
        case OP_NEG: return "-";
    }
}

std::string efd::NDBinOp::toString(bool pretty) const {
    std::string str;

    if (!mChild[I_LHS]->isEmpty())
        str += mChild[I_LHS]->toString(pretty) + " ";

    str += getOperation();
    str += " " + mChild[I_RHS]->toString(pretty);

    return str;
}

// -------------- Unary Operation -----------------
efd::NDUnaryOp::NDUnaryOp(UOpType t) : Node(false), mT(t) {
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

std::string efd::NDUnaryOp::getOperation() const {
    switch (mT) {
        case UOP_SIN:   return "sin";
        case UOP_COS:   return "con";
        case UOP_TAN:   return "tan";
        case UOP_EXP:   return "exp";
        case UOP_LN:    return "ln";
        case UOP_SQRT:  return "sqrt";
    }
}

std::string efd::NDUnaryOp::toString(bool pretty) const {
    std::string str;

    str += getOperation();
    str += "(" + mChild[I_ONLY]->toString(pretty) + ")";

    return str;
}

// -------------- ID reference Operation -----------------
efd::NDIdRef::NDIdRef() : Node(false) {
}

std::string efd::NDIdRef::toString(bool pretty) const {
    std::string str;

    str += mChild[I_ID]->toString(pretty);
    str += "[" + mChild[I_N]->toString(pretty) + "]";

    return str;
}

// -------------- Arg list Operation -----------------
efd::NDArgList::NDArgList() : Node(false) {
}

std::string efd::NDArgList::toString(bool pretty) const {
    std::string str;

    if (!mChild.empty()) {
        str += mChild[0]->toString(pretty);

        for (auto &arg : *this)
            str += ", " + arg->toString(pretty);
    }

    return str;
}
