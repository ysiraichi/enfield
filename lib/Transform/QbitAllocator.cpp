#include "enfield/Transform/QbitAllocator.h"
#include "enfield/Transform/Utils.h"

#include <iterator>
#include <cassert>

void efd::QbitAllocator::updateDepSet() {
    mDepSet = mDepPass->getDependencies();
}

efd::QbitAllocator::QbitAllocator(QModule* qmod, Graph* pGraph, SwapFinder* sFind, 
        DependencyBuilderPass* depPass) : mMod(qmod), mPhysGraph(pGraph), 
                                          mSFind(sFind), mDepPass(depPass), mRun(false) {
    if (mDepPass == nullptr)
        mDepPass = DependencyBuilderPass::Create(mMod);
}

efd::QbitAllocator::Iterator efd::QbitAllocator::inlineDep(Iterator it) {
    Iterator newIt = it;

    if (it->mCallPoint != nullptr) {
        unsigned dist = std::distance(mDepSet.begin(), it);

        efd::InlineGate(mMod, it->mCallPoint);
        mMod->runPass(mDepPass, true);
        updateDepSet();

        newIt = mDepSet.begin() + dist;
    }

    return newIt;
}

void efd::QbitAllocator::run() {
    mMod->runPass(mDepPass);
    updateDepSet();

    mMapping = generateMapping(mDepSet);
    mRun = true;
}

efd::QbitAllocator::Mapping efd::QbitAllocator::getMapping() {
    assert(mRun && "You have to run the allocator before getting the mapping.");
    return mMapping;
}
