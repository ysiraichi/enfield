#include "enfield/Transform/InlineAllPass.h"

efd::InlineAllPass::InlineAllPass(QModule::sRef qmod, std::vector<std::string> basis) :
    mMod(qmod), mInlined(false) {

    mBasis = std::set<std::string>(basis.begin(), basis.end());
    mUK += Pass::K_STMT_PASS;
}

void efd::InlineAllPass::initImpl(bool force) {
    mInlined = false;
}

void efd::InlineAllPass::recursiveInline(NDQOpGeneric::Ref ref) {
}

void efd::InlineAllPass::visit(NDQOpGeneric::Ref ref) {
    if (mBasis.find(ref->getId()->getVal()) == mBasis.end()) {
        mMod->inlineCall(ref);
        mInlined = true;
    }
}

void efd::InlineAllPass::visit(NDIfStmt::Ref ref) {
    ref->getQOp()->apply(this);
}

bool efd::InlineAllPass::hasInlined() const {
    return mInlined;
}

bool efd::InlineAllPass::doesInvalidatesModule() const {
    return true;
}

efd::InlineAllPass::uRef efd::InlineAllPass::Create(QModule::sRef qmod, std::vector<std::string> basis) {
    return uRef(new InlineAllPass(qmod, basis));
}
