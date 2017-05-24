#include "enfield/Transform/QbitAllocator.h"
#include "enfield/Transform/Utils.h"
#include "enfield/Support/RTTI.h"

#include <iterator>
#include <cassert>

void efd::QbitAllocator::updateDepSet() {
    mDepSet = mDepPass->getDependencies();
}

efd::NodeRef efd::QbitAllocator::getIdNodeFromString(std::string s) {
    NodeRef node = nullptr;

    std::size_t idEnd = s.find("[");
    if (idEnd == std::string::npos) {
        // Is a NDId
        node = NDId::Create(s);
    } else {
        // Is a NDIdRef
        std::string id = s.substr(0, idEnd);
        std::size_t szEnd = s.find("]");
        std::string size = s.substr(idEnd+1, szEnd - idEnd - 1);
        node = NDIdRef::Create(NDId::Create(id), NDInt::Create(size));
    }

    return node;
}

efd::QbitAllocator::QbitAllocator(QModule* qmod, Graph* pGraph, SwapFinder* sFind, 
        DependencyBuilderPass* depPass) : mMod(qmod), mPhysGraph(pGraph), 
                                          mSFind(sFind), mDepPass(depPass), mRun(false) {
    if (mDepPass == nullptr)
        mDepPass = DependencyBuilderPass::Create(mMod);
}

efd::QbitAllocator::Iterator efd::QbitAllocator::inlineDep(Iterator it) {
    Iterator newIt = it;

    if (NDQOpGeneric* refCall = dynCast<NDQOpGeneric>(it->mCallPoint)) {
        unsigned dist = std::distance(mDepSet.begin(), it);

        efd::InlineGate(mMod, refCall);
        mMod->runPass(mDepPass, true);
        updateDepSet();

        newIt = mDepSet.begin() + dist;
    }

    return newIt;
}

void efd::QbitAllocator::insertSwapBefore(Dependencies& deps, unsigned u, unsigned v) {
    QbitToNumberPass* qbitPass = mDepPass->getUIdPass();
    NodeRef lhs = getIdNodeFromString(qbitPass->getStrId(u));
    NodeRef rhs = getIdNodeFromString(qbitPass->getStrId(v));
    InsertSwapBefore(deps.mCallPoint, lhs, rhs);
}

void efd::QbitAllocator::run() {
    mMod->runPass(mDepPass);
    updateDepSet();

    mMapping = solveDependencies(mDepSet);
    mRun = true;
}

efd::QbitAllocator::Mapping efd::QbitAllocator::getMapping() {
    assert(mRun && "You have to run the allocator before getting the mapping.");
    return mMapping;
}
