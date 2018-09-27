#include "enfield/Transform/Allocators/QbitAllocator.h"
#include "enfield/Transform/RenameQbitsPass.h"
#include "enfield/Transform/InlineAllPass.h"
#include "enfield/Transform/PassCache.h"
#include "enfield/Arch/ArchGraph.h"
#include "enfield/Analysis/NodeVisitor.h"
#include "enfield/Support/RTTI.h"
#include "enfield/Support/uRefCast.h"
#include "enfield/Support/Timer.h"

#include <iterator>

using namespace efd;

static Stat<uint32_t> DepStat
("Dependencies", "The number of dependencies of this program.");
static Stat<double> AllocTime
("AllocTime", "Time to allocate all qubits.");
static Stat<double> InlineTime
("InlineTime", "Time to inline all gates.");
static Stat<double> ReplaceTime
("ReplaceTime", "Time to replace all qubits to the corresponding architechture ones.");
static Stat<double> RenameTime
("RenameTime", "Time to rename all qubits to the mapped qubits.");

InverseMap efd::InvertMapping(uint32_t archQ, Mapping mapping, bool fill) {
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

Mapping efd::IdentityMapping(uint32_t progQ) {
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
QbitAllocator::QbitAllocator(ArchGraph::sRef archGraph) : mArchGraph(archGraph) {
    mGateWeightMap = { {"U", 1}, {"CX", 10} };
}

// In order for calculatin the cost of a CNOT and a Haddamard gate in the most
// generic way possible, we create a program with only one application of the
// RevCNOT gate (it uses both CNOT and Hadammard).
// Then, we inline it based on the given gates and compute the cost of each
// gate, knowing that the only single-qubit gate is a Hadammard, and the only
// two-qubit gate is a CNOT.
void QbitAllocator::calculateHAndCXCost() {
    static const std::string revCXProgram =
"\
OPENQASM 2.0;\
include \"qelib1.inc\";\
qreg q[2];\
intrinsic_rev_cx__ q[0], q[1];\
";
    auto qmod = QModule::ParseString(revCXProgram);
    inlineAllGates(qmod.get());

    for (auto it = qmod->stmt_begin(), end = qmod->stmt_end(); it != end; ++it) {
        auto qopNode = dynCast<NDQOp>(it->get());
        auto qargs = qopNode->getQArgs();
        auto cost = mGateWeightMap[qopNode->getOperation()];

        if (qargs->getChildNumber() == 2) mCXCost = cost;
        else mHCost = cost;
    }
}

void QbitAllocator::inlineAllGates(QModule::Ref qmod) {
    auto inlinePass = InlineAllPass::Create(ExtractGateNames(mGateWeightMap));
    PassCache::Run(qmod, inlinePass.get());
}

void QbitAllocator::replaceWithArchSpecs(QModule::Ref qmod) {
    // Renaming program qbits to architecture qbits.
    RenameQbitPass::ArchMap toArchMap;

    auto xtn = PassCache::Get<XbitToNumberWrapperPass>(qmod);
    auto xbitToNumber = xtn->getData();

    for (uint32_t i = 0, e = xbitToNumber.getQSize(); i < e; ++i) {
        toArchMap[xbitToNumber.getQStrId(i)] = mArchGraph->getNode(i);
    }

    auto renamePass = RenameQbitPass::Create(toArchMap);
    PassCache::Run(qmod, renamePass.get());

    // Replacing the old qbit declarations with the architecture's qbit
    // declaration.
    qmod->removeAllQRegs();
    for (auto it = mArchGraph->reg_begin(), e = mArchGraph->reg_end(); it != e; ++it)
        qmod->insertReg(NDRegDecl::CreateQ
                (NDId::Create(it->first), NDInt::Create(std::to_string(it->second))));
}

uint32_t QbitAllocator::getCXCost(uint32_t u, uint32_t v) {
    if (mArchGraph->hasEdge(u, v)) return mCXCost;
    if (mArchGraph->hasEdge(v, u)) return mCXCost + (4 * mHCost);

    EfdAbortIf(true, "There is no edge (" << u << ", " << v << ") in the architecture graph.");
}

uint32_t QbitAllocator::getSwapCost(uint32_t u, uint32_t v) {
    uint32_t uvCost = getCXCost(u, v);
    uint32_t vuCost = getCXCost(v, u);
    return (uvCost < vuCost) ? (uvCost * 2) + vuCost : (vuCost * 2) + uvCost;
}

uint32_t QbitAllocator::getBridgeCost(uint32_t u, uint32_t w, uint32_t v) {
    return (getCXCost(u, w) * 2) + (getCXCost(w, v) * 2);
}

bool QbitAllocator::run(QModule::Ref qmod) {
    Timer timer;

    // Before running the allocator, we first calculate the costs for CX
    // and H with the gate weights set up.
    calculateHAndCXCost();

    // Setting up timer ----------------
    timer.start();
    // ---------------------------------

    inlineAllGates(qmod);

    // Stopping timer and setting the stat -----------------
    timer.stop();
    InlineTime = ((double) timer.getMicroseconds() / 1000000.0);
    // -----------------------------------------------------

    // Replacing all declared registers to the registers declared in the
    // architecture graph.
    // Setting up timer ----------------
    timer.start();
    // ---------------------------------

    replaceWithArchSpecs(qmod);

    // Stopping timer and setting the stat -----------------
    timer.stop();
    ReplaceTime = ((double) timer.getMicroseconds() / 1000000.0);
    // -----------------------------------------------------

    // Getting the new information, since it can be the case that the qmodule
    // was modified.
    auto depPass = PassCache::Get<DependencyBuilderWrapperPass>(qmod);
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

    mData = allocate(qmod);

    // Stopping timer and setting the stat -----------------
    timer.stop();
    AllocTime = ((double) timer.getMicroseconds() / 1000000.0);
    // -----------------------------------------------------

    INF << "Initial Configuration: " << MappingToString(mData) << std::endl;
    return true;
}

void QbitAllocator::setGateWeightMap(const GateWeightMap& weightMap) {
    mGateWeightMap = weightMap;
}
