#include "enfield/Transform/QbitAllocator.h"
#include "enfield/Transform/RenameQbitsPass.h"
#include "enfield/Transform/Utils.h"
#include "enfield/Arch/ArchGraph.h"
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

        mMod->inlineCall(refCall);
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

    NodeRef parent = deps.mCallPoint->getParent();
    auto it = parent->findChild(deps.mCallPoint);
    mMod->insertSwapBefore(it, lhs, rhs);
}

unsigned efd::QbitAllocator::getNumQbits() {
    return mDepPass->getUIdPass()->getSize();
}

void efd::QbitAllocator::run() {
    if (ArchGraph* arch = dynCast<ArchGraph>(mArchGraph)) {
        RenameQbitPass::ArchMap toArchMap;

        mMod->runPass(mDepPass);
        QbitToNumberPass* uidPass = mDepPass->getUIdPass();
        for (unsigned i = 0, e = uidPass->getSize(); i < e; ++i) {
            toArchMap[uidPass->getStrId(i)] = arch->getNode(i);
        }

        RenameQbitPass* renamePass = RenameQbitPass::Create(toArchMap);
        mMod->runPass(renamePass);

        std::vector<NDDecl*> decls;
        for (auto it = arch->reg_begin(), e = arch->reg_end(); it != e; ++it)
            decls.push_back(dynCast<NDDecl>(NDDecl::Create(NDDecl::QUANTUM, 
                            NDId::Create(it->first), 
                            NDInt::Create(std::to_string(it->second)))
                        ));
        mMod->replaceAllRegsWith(decls);
    }
    
    mMod->runPass(mDepPass);
    updateDepSet();

    mMapping = solveDependencies(mDepSet);

    QbitToNumberPass* uidPass = mDepPass->getUIdPass();

    RenameQbitPass::ArchMap archConstMap;
    if (ArchGraph* arch = dynCast<ArchGraph>(mArchGraph)) {
        for (unsigned i = 0, e = getNumQbits(); i < e; ++i) {
            std::string id = uidPass->getStrId(i);
            archConstMap[id] = arch->getNode(mMapping[i]);
        }
    } else {
        for (unsigned i = 0, e = getNumQbits(); i < e; ++i) {
            std::string id = uidPass->getStrId(i);
            archConstMap[id] = uidPass->getNode(i);
        }
    }

    RenameQbitPass* renamePass = RenameQbitPass::Create(archConstMap);
    mMod->runPass(renamePass);

    mRun = true;
}

efd::QbitAllocator::Mapping efd::QbitAllocator::getMapping() {
    assert(mRun && "You have to run the allocator before getting the mapping.");
    return mMapping;
}
