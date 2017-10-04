#include "enfield/Transform/QbitAllocator.h"
#include "enfield/Transform/RenameQbitsPass.h"
#include "enfield/Transform/InlineAllPass.h"
#include "enfield/Transform/Utils.h"
#include "enfield/Arch/ArchGraph.h"
#include "enfield/Analysis/NodeVisitor.h"
#include "enfield/Support/RTTI.h"
#include "enfield/Support/uRefCast.h"
#include "enfield/Support/Timer.h"

#include <iterator>
#include <cassert>

// ------------------ Solution Implementer ----------------------
namespace efd {
    class SolutionImplPass : public PassT<void>, public NodeVisitor {
        private:
            Solution& mSol;
            XbitToNumberWrapperPass::sRef mXbitToNumberPass;

            XbitToNumber mXbitToNumber;
            std::vector<Node::Ref> mMap;

            std::unordered_map<Node::Ref, std::vector<Node::uRef>> mReplVector;

            unsigned mDepIdx;

            /// \brief Gets the node mapped to the string version of \p ref.
            Node::uRef getMappedNode(Node::Ref ref);
            /// \brief Wraps \p ref with \p ifstmt if \p ifstmt is not nullptr.
            Node::uRef wrapWithIfNode(Node::uRef ref, NDIfStmt::Ref ifstmt);
            /// \brief Apply the swaps for the current dependency.
            void applyOperations(Node::Ref ref);

        public:
            SolutionImplPass(Solution& sol) : mSol(sol), mXbitToNumberPass(nullptr) {}

            void setXbitToNumberPass(XbitToNumberWrapperPass::sRef pass);

            bool run(QModule::Ref qmod) override;
            void visit(NDQOpMeasure::Ref ref) override;
            void visit(NDQOpReset::Ref ref) override;
            void visit(NDQOpU::Ref ref) override;
            void visit(NDQOpBarrier::Ref ref) override;
            void visit(NDIfStmt::Ref ref) override;
            void visit(NDList::Ref ref) override;
            void visit(NDQOpCX::Ref ref) override;
            void visit(NDQOp::Ref ref) override;
    };
}

efd::Node::uRef efd::SolutionImplPass::getMappedNode(Node::Ref ref) {
    unsigned id = mXbitToNumber.getQUId(ref->toString());
    return mMap[id]->clone();
}

efd::Node::uRef efd::SolutionImplPass::wrapWithIfNode
(Node::uRef ref, NDIfStmt::Ref ifstmt) {

    if (ifstmt != nullptr) {
        auto ifclone = uniqueCastForward<NDIfStmt>(ifstmt->clone());
        ifclone->setQOp(uniqueCastForward<NDQOp>(std::move(ref)));
        ref = std::move(ifclone);
    }

    return ref;
}

void efd::SolutionImplPass::applyOperations(Node::Ref ref) {
    auto& ops = mSol.mOpSeqs[mDepIdx].second;
    ++mDepIdx;

    // If there is nothing to be done in this dep, simply return.
    if (ops.empty()) return;

    NDIfStmt::Ref ifstmt = efd::dynCast<NDIfStmt>(ref->getParent());

    Node::Ref key;
    if (ifstmt != nullptr) {
        key = ifstmt;
    } else {
        key = ref;
    }

    mReplVector[key] = std::vector<Node::uRef>();

    for (auto& op : ops) {
        switch (op.mK) {
            case Operation::K_OP_CNOT:
                {
                    NDQOp::uRef clone = uniqueCastForward<NDQOp>(ref->clone());

                    auto qargs = clone->getQArgs();
                    qargs->setChild(0, mMap[op.mU]->clone());
                    qargs->setChild(1, mMap[op.mV]->clone());

                    mReplVector[key].push_back(std::move(clone));
                }
                break;

            case Operation::K_OP_SWAP:
                mReplVector[key].push_back(
                        efd::CreateISwap(mMap[op.mU]->clone(), mMap[op.mV]->clone()));
                std::swap(mMap[op.mU], mMap[op.mV]);
                break;

            case Operation::K_OP_REV:
                {
                    Node::uRef call = efd::CreateIRevCX(
                                mMap[op.mU]->clone(),
                                mMap[op.mV]->clone());
                    call = wrapWithIfNode(std::move(call), ifstmt);
                    mReplVector[key].push_back(wrapWithIfNode(std::move(call), ifstmt));
                }
                break;

            case Operation::K_OP_LCNOT:
                {
                    Node::uRef call = efd::CreateILongCX(
                                mMap[op.mU]->clone(), 
                                mMap[op.mW]->clone(), 
                                mMap[op.mV]->clone());
                    call = wrapWithIfNode(std::move(call), ifstmt);
                    mReplVector[key].push_back(wrapWithIfNode(std::move(call), ifstmt));
                }
                break;
        }
    }
}

void efd::SolutionImplPass::setXbitToNumberPass(XbitToNumberWrapperPass::sRef pass) {
    mXbitToNumberPass = pass;
}

bool efd::SolutionImplPass::run(QModule::Ref qmod) {
    if (mXbitToNumberPass.get() == nullptr) {
        mXbitToNumberPass = XbitToNumberWrapperPass::Create();
        mXbitToNumberPass->run(qmod);
    }

    mXbitToNumber = mXbitToNumberPass->getData();
    mMap.assign(mXbitToNumber.getQSize(), nullptr);

    for (unsigned i = 0, e = mXbitToNumber.getQSize(); i < e; ++i)
        mMap[i] = mXbitToNumber.getQNode(mSol.mInitial[i]);

    mDepIdx = 0;
    for (auto it = qmod->stmt_begin(), end = qmod->stmt_end(); it != end; ++it) {
        (*it)->apply(this);
    }

    for (auto& pair : mReplVector) {
        if (!pair.second.empty())
            qmod->replaceStatement(pair.first, std::move(pair.second));
    }

    return true;
}

void efd::SolutionImplPass::visit(NDQOpMeasure::Ref ref) {
    ref->setQBit(getMappedNode(ref->getQBit()));
}

void efd::SolutionImplPass::visit(NDQOpReset::Ref ref) {
    ref->setQArg(getMappedNode(ref->getQArg()));
}

void efd::SolutionImplPass::visit(NDQOpU::Ref ref) {
    ref->setQArg(getMappedNode(ref->getQArg()));
}

void efd::SolutionImplPass::visit(NDQOpBarrier::Ref ref) {
    ref->getQArgs()->apply(this);
}

void efd::SolutionImplPass::visit(NDIfStmt::Ref ref) {
    ref->getQOp()->apply(this);
}

void efd::SolutionImplPass::visit(NDList::Ref ref) {
    for (unsigned i = 0, e = ref->getChildNumber(); i < e; ++i) {
        ref->setChild(i, getMappedNode(ref->getChild(i)));
    }
}

void efd::SolutionImplPass::visit(NDQOpCX::Ref ref) {
    ref->setLhs(getMappedNode(ref->getLhs()));
    ref->setRhs(getMappedNode(ref->getRhs()));

    assert(ref == mSol.mOpSeqs[mDepIdx].first &&
            "Wrong cnot dependency.");

    applyOperations(ref);
}

void efd::SolutionImplPass::visit(NDQOp::Ref ref) {
    ref->getQArgs()->apply(this);

    if (!mSol.mOpSeqs.empty() && ref == mSol.mOpSeqs[mDepIdx].first) {
        applyOperations(ref);
    }
}


// ------------------ QbitAllocator ----------------------
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
    mXbitToNumber = mDepBuilder.getXbitToNumber();
}

efd::QbitAllocator::Iterator efd::QbitAllocator::inlineDep(QbitAllocator::Iterator it) {
    Iterator newIt = it;

    if (NDQOp::Ref refCall = dynCast<NDQOp>(it->mCallPoint)) {
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

    auto xtn = XbitToNumberWrapperPass::Create();
    xtn->run(mMod);

    auto xbitToNumber = xtn->getData();
    for (unsigned i = 0, e = xbitToNumber.getQSize(); i < e; ++i) {
        toArchMap[xbitToNumber.getQStrId(i)] = mArchGraph->getNode(i);
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
    auto xtn = XbitToNumberWrapperPass::Create();
    xtn->run(mMod);

    auto xbitToNumber = xtn->getData();
    // Renaming the qbits with the mapping that this algorithm got from solving
    // the dependencies.
    RenameQbitPass::ArchMap archConstMap;
    if (!mArchGraph->isGeneric()) {
        for (unsigned i = 0, e = xbitToNumber.getQSize(); i < e; ++i) {
            std::string id = xbitToNumber.getQStrId(i);
            archConstMap[id] = mArchGraph->getNode(mSol.mInitial[i]);
        }
    } else {
        for (unsigned i = 0, e = xbitToNumber.getQSize(); i < e; ++i) {
            std::string id = xbitToNumber.getQStrId(i);
            archConstMap[id] = xbitToNumber.getQNode(mSol.mInitial[i]);
        }
    }

    auto renamePass = RenameQbitPass::Create(archConstMap);
    renamePass->run(mMod);
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

    TotalCost = mSol.mCost;

    // Setting up timer ----------------
    timer.start();
    // ---------------------------------

    SolutionImplPass pass(mSol);
    pass.run(mMod);
    // renameQbits();

    // Stopping timer and setting the stat -----------------
    timer.stop();
    RenameTime = ((double) timer.getMicroseconds() / 1000000.0);
    // -----------------------------------------------------

    return true;
}

unsigned efd::QbitAllocator::getNumQbits() {
    return mXbitToNumber.getQSize();
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
