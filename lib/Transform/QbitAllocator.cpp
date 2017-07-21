
#include "enfield/Transform/QbitAllocator.h"
#include "enfield/Transform/RenameQbitsPass.h"
#include "enfield/Transform/InlineAllPass.h"
#include "enfield/Transform/Utils.h"
#include "enfield/Arch/ArchGraph.h"
#include "enfield/Support/RTTI.h"
#include "enfield/Support/Timer.h"

#include <iterator>
#include <cassert>

static efd::Stat<unsigned> DepStat
("Dependencies", "The number of dependencies of this program.");
static efd::Stat<double> AllocTime
("AllocTime", "Time to allocate all qubits.");
static efd::Stat<double> InlineTime
("InlineTime", "Time to inline all gates.");
static efd::Stat<double> ReplaceTime
("ReplaceTime", "Time to replace all qubits to the corresponding architechture ones.");
static efd::Stat<double> RenameTime
("RenameTime", "Time to rename all qubits to the mapped qubits.");

efd::Stat<unsigned> TotalCost
("TotalCost", "Total cost after allocating the qubits.");
efd::Opt<unsigned> SwapCost
("-swap-cost", "Cost of using a swap function.", 7, false);
efd::Opt<unsigned> RevCost
("-rev-cost", "Cost of using a reverse edge.", 4, false);

void efd::QbitAllocator::updateDepSet() {
    mDepSet = mDepPass->getDependencies();
}

efd::QbitAllocator::QbitAllocator(QModule::sRef qmod, ArchGraph::sRef archGraph) 
    : mMod(qmod), mArchGraph(archGraph), mRun(false), mInlineAll(false) {
    mDepPass = DependencyBuilderPass::sRef(DependencyBuilderPass::Create(mMod).release());
}

efd::QbitAllocator::Iterator efd::QbitAllocator::inlineDep(Iterator it) {
    Iterator newIt = it;

    if (NDQOpGeneric::Ref refCall = dynCast<NDQOpGeneric>(it->mCallPoint)) {
        unsigned dist = std::distance(mDepSet.begin(), it);

        mMod->inlineCall(refCall);
        mMod->runPass(mDepPass.get(), true);
        updateDepSet();

        newIt = mDepSet.begin() + dist;
    }

    return newIt;
}

void efd::QbitAllocator::insertSwapBefore(Dependencies& deps, unsigned u, unsigned v) {
    QbitToNumberPass::sRef qbitPass = mDepPass->getUIdPass();
    Node::Ref lhs = qbitPass->getNode(u);
    Node::Ref rhs = qbitPass->getNode(v);

    Node::Ref parent = deps.mCallPoint->getParent();
    auto it = parent->findChild(deps.mCallPoint);
    mMod->insertSwapBefore(it, lhs, rhs);
}

unsigned efd::QbitAllocator::getNumQbits() {
    return mDepPass->getUIdPass()->getSize();
}

void efd::QbitAllocator::inlineAllGates() {
    auto inlinePass = InlineAllPass::Create(mMod, mBasis);

    do {
        mMod->runPass(inlinePass.get(), true);
    } while (inlinePass->hasInlined());
}

void efd::QbitAllocator::replaceWithArchSpecs() {
    // Renaming program qbits to architecture qbits.
    RenameQbitPass::ArchMap toArchMap;

    mMod->runPass(mDepPass.get());
    QbitToNumberPass::sRef uidPass = mDepPass->getUIdPass();
    for (unsigned i = 0, e = uidPass->getSize(); i < e; ++i) {
        toArchMap[uidPass->getStrId(i)] = mArchGraph->getNode(i);
    }

    auto renamePass = RenameQbitPass::Create(toArchMap);
    mMod->runPass(renamePass.get());

    // Replacing the old qbit declarations with the architecture's qbit
    // declaration.
    std::vector<NDRegDecl::uRef> decls;
    for (auto it = mArchGraph->reg_begin(), e = mArchGraph->reg_end(); it != e; ++it)
        decls.push_back(NDRegDecl::CreateQ(
                        NDId::Create(it->first), 
                        NDInt::Create(std::to_string(it->second))));
    mMod->replaceAllRegsWith(std::move(decls));
}

void efd::QbitAllocator::renameQbits() {
    QbitToNumberPass::sRef uidPass = mDepPass->getUIdPass();

    // Renaming the qbits with the mapping that this algorithm got from solving
    // the dependencies.
    RenameQbitPass::ArchMap archConstMap;
    if (!mArchGraph->isGeneric()) {
        for (unsigned i = 0, e = uidPass->getSize(); i < e; ++i) {
            std::string id = uidPass->getStrId(i);
            archConstMap[id] = mArchGraph->getNode(mMapping[i]);
        }
    } else {
        for (unsigned i = 0, e = uidPass->getSize(); i < e; ++i) {
            std::string id = uidPass->getStrId(i);
            archConstMap[id] = uidPass->getNode(mMapping[i]);
        }
    }

    auto renamePass = RenameQbitPass::Create(archConstMap);
    mMod->runPass(renamePass.get());
}

void efd::QbitAllocator::run() {
    Timer timer;

    if (mInlineAll) {
        // Setting up timer ----------------
        timer.start();
        // ---------------------------------

        inlineAllGates();

        // Stopping timer and setting the stat -----------------
        timer.stop();
        InlineTime = ((double) timer.getMicroseconds() / 1000000.0);
        // -----------------------------------------------------
    }

    if (!mArchGraph->isGeneric()) {
        // Setting up timer ----------------
        timer.start();
        // ---------------------------------

        replaceWithArchSpecs();

        // Stopping timer and setting the stat -----------------
        timer.stop();
        ReplaceTime = ((double) timer.getMicroseconds() / 1000000.0);
        // -----------------------------------------------------
    }

    // Getting the new information, since it can be the case that the qmodule
    // was modified.
    mMod->runPass(mDepPass.get(), true);
    updateDepSet();

    // Counting total dependencies.
    unsigned totalDeps = 0;
    for (auto& d : mDepSet)
        totalDeps += d.mDeps.size();
    DepStat = totalDeps;

    // Setting up timer ----------------
    timer.start();
    // ---------------------------------

    mMapping = solveDependencies(mDepSet);

    // Stopping timer and setting the stat -----------------
    timer.stop();
    AllocTime = ((double) timer.getMicroseconds() / 1000000.0);
    // -----------------------------------------------------

    // Setting up timer ----------------
    timer.start();
    // ---------------------------------

    renameQbits();

    // Stopping timer and setting the stat -----------------
    timer.stop();
    RenameTime = ((double) timer.getMicroseconds() / 1000000.0);
    // -----------------------------------------------------

    mRun = true;
}

void efd::QbitAllocator::setInlineAll(BasisVector basis) {
    mInlineAll = true;
    mBasis = basis;
}

void efd::QbitAllocator::setDontInline() {
    mInlineAll = false;
}

efd::QbitAllocator::Mapping efd::QbitAllocator::getMapping() {
    assert(mRun && "You have to run the allocator before getting the mapping.");
    return mMapping;
}
