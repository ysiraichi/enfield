#include "enfield/Transform/QbitAllocator.h"
#include "enfield/Transform/RenameQbitsPass.h"
#include "enfield/Transform/InlineAllPass.h"
#include "enfield/Transform/Utils.h"
#include "enfield/Arch/ArchGraph.h"
#include "enfield/Support/RTTI.h"
#include "enfield/Support/uRefCast.h"
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
efd::Opt<unsigned> LCXCost
("-lcx-cost", "Cost of using long cnot gate.", 10, false);

efd::QbitAllocator::QbitAllocator(ArchGraph::sRef archGraph) 
    : mArchGraph(archGraph), mInlineAll(false) {
}

void efd::QbitAllocator::updateDependencies() {
    auto depPass = DependencyBuilderWrapperPass::Create();
    depPass->run(mMod);

    mDepBuilder = depPass->getData();
    mQbitToNumber = mDepBuilder.getQbitToNumber();
}

efd::QbitAllocator::Iterator efd::QbitAllocator::inlineDep(QbitAllocator::Iterator it) {
    Iterator newIt = it;

    if (NDQOpGeneric::Ref refCall = dynCast<NDQOpGeneric>(it->mCallPoint)) {
        auto& deps = mDepBuilder.getDependencies();
        unsigned dist = std::distance(deps.begin(), it);

        mMod->inlineCall(refCall);
        newIt = deps.begin() + dist;
    }

    return newIt;
}

void efd::QbitAllocator::inlineAllGates() {
    auto inlinePass = InlineAllPass::Create(mBasis);
    inlinePass->run(mMod);
}

void efd::QbitAllocator::replaceWithArchSpecs() {
    // Renaming program qbits to architecture qbits.
    RenameQbitPass::ArchMap toArchMap;

    auto qtn = QbitToNumberWrapperPass::Create();
    qtn->run(mMod);

    auto qbitToNumber = qtn->getData();
    for (unsigned i = 0, e = qbitToNumber.getSize(); i < e; ++i) {
        toArchMap[qbitToNumber.getStrId(i)] = mArchGraph->getNode(i);
    }

    auto renamePass = RenameQbitPass::Create(toArchMap);
    renamePass->run(mMod);

    // Replacing the old qbit declarations with the architecture's qbit
    // declaration.
    mMod->removeAllQRegs();
    for (auto it = mArchGraph->reg_begin(), e = mArchGraph->reg_end(); it != e; ++it)
        mMod->insertReg(NDRegDecl::CreateQ
                (NDId::Create(it->first), NDInt::Create(std::to_string(it->second))));
}

void efd::QbitAllocator::renameQbits() {
    auto qtn = QbitToNumberWrapperPass::Create();
    qtn->run(mMod);

    auto qbitToNumber = qtn->getData();
    // Renaming the qbits with the mapping that this algorithm got from solving
    // the dependencies.
    RenameQbitPass::ArchMap archConstMap;
    if (!mArchGraph->isGeneric()) {
        for (unsigned i = 0, e = qbitToNumber.getSize(); i < e; ++i) {
            std::string id = qbitToNumber.getStrId(i);
            archConstMap[id] = mArchGraph->getNode(mSol.mInitial[i]);
        }
    } else {
        for (unsigned i = 0, e = qbitToNumber.getSize(); i < e; ++i) {
            std::string id = qbitToNumber.getStrId(i);
            archConstMap[id] = qbitToNumber.getNode(mSol.mInitial[i]);
        }
    }

    auto renamePass = RenameQbitPass::Create(archConstMap);
    renamePass->run(mMod);
}

void efd::QbitAllocator::run(QModule::Ref qmod) {
    Timer timer;

    // Setting the class QModule.
    mMod = qmod;

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
    updateDependencies();
    auto& deps = mDepBuilder.getDependencies();

    // Counting total dependencies.
    unsigned totalDeps = 0;
    for (auto& d : deps)
        totalDeps += d.mDeps.size();
    DepStat = totalDeps;

    // Setting up timer ----------------
    timer.start();
    // ---------------------------------

    mSol = solve(deps);

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
}

unsigned efd::QbitAllocator::getNumQbits() {
    return mQbitToNumber.getSize();
}

void efd::QbitAllocator::setInlineAll(BasisVector basis) {
    mInlineAll = true;
    mBasis = basis;
}

void efd::QbitAllocator::setDontInline() {
    mInlineAll = false;
}

efd::QbitAllocator::Mapping efd::GenAssignment
(unsigned archQ, QbitAllocator::Mapping mapping) {
    // 'archQ' is the number of qubits from the architecture.
    std::vector<unsigned> assign(archQ, archQ);

    // for 'u' in arch; and 'a' in prog:
    // if 'a' -> 'u', then 'u' -> 'a'
    for (unsigned i = 0, e = mapping.size(); i < e; ++i)
        assign[mapping[i]] = i;

    // Fill the qubits in the architecture that were not mapped.
    unsigned id = mapping.size();
    for (unsigned i = 0; i < archQ; ++i)
        assign[i] = (assign[i] == archQ) ? id++ : assign[i];

    return assign;
}
