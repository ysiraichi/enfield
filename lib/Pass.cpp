#include "enfield/Pass.h"

efd::Pass::Pass() : mApplied(false), mUK(0) {
}

bool efd::Pass::isGatePass() const {
    return mUK & K_GATE_PASS;
}

bool efd::Pass::isRegDeclPass() const {
    return mUK & K_REG_DECL_PASS;
}

bool efd::Pass::isStatementPass() const {
    return mUK & K_STMT_PASS;
}

bool efd::Pass::isASTPass() const {
    return mUK & K_AST_PASS;
}

bool efd::Pass::wasApplied() const {
    return mApplied;
}

void efd::Pass::initImpl(bool force) {
}

void efd::Pass::init(bool force) {
    initImpl(force);
    mApplied = true;
}

bool efd::Pass::doesInvalidatesModule() const {
    return false;
}
