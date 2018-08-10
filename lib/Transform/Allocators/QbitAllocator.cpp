#include "enfield/Transform/Allocators/QbitAllocator.h"
#include "enfield/Transform/RenameQbitsPass.h"
#include "enfield/Transform/InlineAllPass.h"
#include "enfield/Transform/PassCache.h"
#include "enfield/Transform/Utils.h"
#include "enfield/Arch/ArchGraph.h"
#include "enfield/Analysis/NodeVisitor.h"
#include "enfield/Support/RTTI.h"
#include "enfield/Support/uRefCast.h"
#include "enfield/Support/Timer.h"

#include <iterator>

static efd::Stat<uint32_t> DepStat
("Dependencies", "The number of dependencies of this program.");
static efd::Stat<double> AllocTime
("AllocTime", "Time to allocate all qubits.");
static efd::Stat<double> InlineTime
("InlineTime", "Time to inline all gates.");
static efd::Stat<double> ReplaceTime
("ReplaceTime", "Time to replace all qubits to the corresponding architechture ones.");
static efd::Stat<double> RenameTime
("RenameTime", "Time to rename all qubits to the mapped qubits.");

efd::InverseMap efd::InvertMapping(uint32_t archQ, Mapping mapping, bool fill) {
    uint32_t progQ = mapping.size();
    // 'archQ' is the number of qubits from the architecture.
    std::vector<uint32_t> inv(archQ, _undef);

    // for 'u' in arch; and 'a' in prog:
    // if 'a' -> 'u', then 'u' -> 'a'
    for (uint32_t i = 0; i < progQ; ++i)
        if (mapping[i] != _undef)
            inv[mapping[i]] = i;

    if (fill) {
        // Fill the qubits in the architecture that were not mapped.
        Fill(mapping, inv);
    }

    return inv;
}

void efd::Fill(Mapping& mapping, InverseMap& inv) {
    uint32_t progQ = mapping.size(), archQ = inv.size();
    uint32_t a = 0, u = 0;

    do {
        while (a < progQ && mapping[a] != _undef) ++a;
        while (u < archQ && inv[u] != _undef) ++u;

        if (u < archQ && a < progQ) {
            mapping[a] = u;
            inv[u] = a;
            ++u; ++a;
        } else {
            break;
        }
    } while (true);
}

void efd::Fill(uint32_t archQ, Mapping& mapping) {
    auto inv = InvertMapping(archQ, mapping, false);
    Fill(mapping, inv);
}

efd::Mapping efd::IdentityMapping(uint32_t progQ) {
    Mapping mapping(progQ, _undef);

    for (uint32_t i = 0; i < progQ; ++i) {
        mapping[i] = i;
    }

    return mapping;
}

std::string efd::MappingToString(Mapping m) {
    std::string s = "[";
    for (uint32_t i = 0, e = m.size(); i < e; ++i) {
        s = s + std::to_string(i) + " => ";
        if (m[i] == _undef) s = s + "_undef";
        else s = s + std::to_string(m[i]);
        s = s + ";";
        if (i != e - 1) s = s + " ";
    }
    s = s + "]";
    return s;
}

// ------------------ QbitAllocator ----------------------
efd::QbitAllocator::QbitAllocator(ArchGraph::sRef archGraph) 
    : mInlineAll(false), mArchGraph(archGraph) {
}

void efd::QbitAllocator::inlineAllGates() {
    auto inlinePass = InlineAllPass::Create(mBasis);
    PassCache::Run(mMod, inlinePass.get());
}

void efd::QbitAllocator::replaceWithArchSpecs() {
    // Renaming program qbits to architecture qbits.
    RenameQbitPass::ArchMap toArchMap;

    auto xtn = PassCache::Get<XbitToNumberWrapperPass>(mMod);
    auto xbitToNumber = xtn->getData();

    for (uint32_t i = 0, e = xbitToNumber.getQSize(); i < e; ++i) {
        toArchMap[xbitToNumber.getQStrId(i)] = mArchGraph->getNode(i);
    }

    auto renamePass = RenameQbitPass::Create(toArchMap);
    PassCache::Run(mMod, renamePass.get());

    // Replacing the old qbit declarations with the architecture's qbit
    // declaration.
    mMod->removeAllQRegs();
    for (auto it = mArchGraph->reg_begin(), e = mArchGraph->reg_end(); it != e; ++it)
        mMod->insertReg(NDRegDecl::CreateQ
                (NDId::Create(it->first), NDInt::Create(std::to_string(it->second))));
}

bool efd::QbitAllocator::run(QModule::Ref qmod) {
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

    // Replacing all declared registers to the registers declared in the
    // architecture graph.
    // Setting up timer ----------------
    timer.start();
    // ---------------------------------

    replaceWithArchSpecs();

    // Stopping timer and setting the stat -----------------
    timer.stop();
    ReplaceTime = ((double) timer.getMicroseconds() / 1000000.0);
    // -----------------------------------------------------

    // Getting the new information, since it can be the case that the qmodule
    // was modified.
    auto depPass = PassCache::Get<DependencyBuilderWrapperPass>(mMod);
    auto depBuilder = depPass->getData();
    auto& deps = depBuilder.getDependencies();

    // Counting total dependencies.
    uint32_t totalDeps = 0;
    for (auto& d : deps) totalDeps += d.mDeps.size();
    DepStat = totalDeps;

    // Filling Qubit information.
    mVQubits = depBuilder.mXbitToNumber.getQSize();
    mPQubits = mArchGraph->size();

    // Setting up timer ----------------
    timer.start();
    // ---------------------------------

    mData = allocate(mMod);

    // Stopping timer and setting the stat -----------------
    timer.stop();
    AllocTime = ((double) timer.getMicroseconds() / 1000000.0);
    // -----------------------------------------------------

    return true;
}

void efd::QbitAllocator::setInlineAll(BasisVector basis) {
    mInlineAll = true;
    mBasis = basis;
}

void efd::QbitAllocator::setDontInline() {
    mInlineAll = false;
}
