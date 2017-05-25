#include "enfield/Transform/QbitAllocator.h"
#include "enfield/Transform/RenameQbitsPass.h"
#include "enfield/Transform/Utils.h"
#include "enfield/Support/RTTI.h"

#include <iterator>
#include <cassert>

void efd::QbitAllocator::updateDepSet() {
    mDepSet = mDepPass->getDependencies();
}

efd::QbitAllocator::QbitAllocator(QModule* qmod, Graph* pGraph, SwapFinder* sFind, 
        DependencyBuilderPass* depPass) : mMod(qmod), mArchGraph(pGraph), 
                                          mSFind(sFind), mDepPass(depPass), mRun(false) {
    if (mDepPass == nullptr)
        mDepPass = DependencyBuilderPass::Create(mMod);
}

efd::QbitAllocator::Iterator efd::QbitAllocator::inlineDep(Iterator it) {
    Iterator newIt = it;

    if (NDQOpGeneric* refCall = dynCast<NDQOpGeneric>(it->mCallPoint)) {
        unsigned dist = std::distance(mDepSet.begin(), it);

        efd::InlineGate(mMod, refCall);
        mMod->invalidate();
        mMod->runPass(mDepPass, true);
        updateDepSet();

        newIt = mDepSet.begin() + dist;
    }

    return newIt;
}

void efd::QbitAllocator::insertSwapBefore(Dependencies& deps, unsigned u, unsigned v) {
    QbitToNumberPass* qbitPass = mDepPass->getUIdPass();
    NodeRef lhs = qbitPass->getNode(u);
    NodeRef rhs = qbitPass->getNode(v);
    InsertSwapBefore(deps.mCallPoint, lhs, rhs);
}

unsigned efd::QbitAllocator::getNumQbits() {
    return mDepPass->getUIdPass()->getSize();
}

void efd::QbitAllocator::run() {
    mMod->runPass(mDepPass);
    updateDepSet();

    mMapping = solveDependencies(mDepSet);
    mMod->invalidate();

    QbitToNumberPass* uidPass = mDepPass->getUIdPass();

    RenameQbitPass::ArchMap map;
    for (unsigned i = 0, e = getNumQbits(); i < e; ++i) {
        std::string id = uidPass->getStrId(i);
        map[id] = uidPass->getNode(i);
    }

    RenameQbitPass* renamePass = RenameQbitPass::Create(map);
    mMod->runPass(renamePass);

    mRun = true;
}

efd::QbitAllocator::Mapping efd::QbitAllocator::getMapping() {
    assert(mRun && "You have to run the allocator before getting the mapping.");
    return mMapping;
}
