#include "enfield/Transform/Allocators/StdSolutionQAllocator.h"
#include "enfield/Transform/RenameQbitsPass.h"
#include "enfield/Transform/InlineAllPass.h"
#include "enfield/Transform/PassCache.h"
#include "enfield/Analysis/NodeVisitor.h"
#include "enfield/Support/RTTI.h"
#include "enfield/Support/uRefCast.h"

using namespace efd;

// ------------------ StdSolution Implementer ----------------------
namespace efd {
    class SolutionImplPass : public PassT<void>, public NodeVisitor {
        private:
            Solution& mData;

            XbitToNumber mXbitToNumber;
            std::vector<Node::Ref> mMap;

            std::unordered_map<Node::Ref, std::vector<Node::uRef>> mReplVector;

            uint32_t mDepIdx;

            /// \brief Gets the node mapped to the string version of \p ref.
            Node::uRef getMappedNode(Node::Ref ref);
            /// \brief Wraps \p ref with \p ifstmt if \p ifstmt is not nullptr.
            Node::uRef wrapWithIfNode(Node::uRef ref, NDIfStmt::Ref ifstmt);
            /// \brief Apply the swaps for the current dependency.
            void applyOperations(NDQOp::Ref ref, NDIfStmt::Ref ifstmt = nullptr);

        public:
            SolutionImplPass(Solution& sol) : mData(sol) {}

            bool run(QModule::Ref qmod) override;
            void visit(NDQOpMeasure::Ref ref) override;
            void visit(NDQOpReset::Ref ref) override;
            void visit(NDQOpU::Ref ref) override;
            void visit(NDQOpBarrier::Ref ref) override;
            void visit(NDIfStmt::Ref ref) override;
            void visit(NDList::Ref ref) override;
            void visit(NDQOpCX::Ref ref) override;
            void visit(NDQOpGen::Ref ref) override;
    };
}

efd::Node::uRef efd::SolutionImplPass::getMappedNode(Node::Ref ref) {
    uint32_t id = mXbitToNumber.getQUId(ref->toString());
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

void efd::SolutionImplPass::applyOperations(NDQOp::Ref qop, NDIfStmt::Ref ifstmt) {
    bool stillHasOpSeqs = mDepIdx < mData.mOpSeqs.size();
    bool isIfStmtAndHasOp = ifstmt && stillHasOpSeqs && ifstmt == mData.mOpSeqs[mDepIdx].first;
    bool isQOpAndHasOp = !ifstmt && stillHasOpSeqs && qop == mData.mOpSeqs[mDepIdx].first;

    if (isIfStmtAndHasOp || isQOpAndHasOp) {
        auto& ops = mData.mOpSeqs[mDepIdx].second;
        ++mDepIdx;

        Node::Ref key = ifstmt;
        if (key == nullptr) { key = qop; }

        mReplVector[key] = std::vector<Node::uRef>();

        for (auto& op : ops) {
            switch (op.mK) {
                case Operation::K_OP_CNOT:
                    {
                        auto clone = uniqueCastForward<NDQOp>(qop->clone());
                        clone->getQArgs()->apply(this);
                        mReplVector[key].push_back(wrapWithIfNode(std::move(clone), ifstmt));
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
                        mReplVector[key].push_back(wrapWithIfNode(std::move(call), ifstmt));
                    }
                    break;

                case Operation::K_OP_LCNOT:
                    {
                        Node::uRef call = efd::CreateILongCX(
                                    mMap[op.mU]->clone(), 
                                    mMap[op.mW]->clone(), 
                                    mMap[op.mV]->clone());
                        mReplVector[key].push_back(wrapWithIfNode(std::move(call), ifstmt));
                    }
                    break;
            }
        }

        if (ops.empty()) {
            qop->getQArgs()->apply(this);
        }
    } else {
        qop->getQArgs()->apply(this);
    }
}

bool efd::SolutionImplPass::run(QModule::Ref qmod) {
    INF << "Initial Configuration: " << MappingToString(mData.mInitial) << std::endl;
    auto xtonpass = PassCache::Get<XbitToNumberWrapperPass>(qmod);

    mXbitToNumber = xtonpass->getData();
    mMap.assign(mXbitToNumber.getQSize(), nullptr);

    for (uint32_t i = 0, e = mXbitToNumber.getQSize(); i < e; ++i) {
        mMap[i] = mXbitToNumber.getQNode(mData.mInitial[i]);
    }

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
    applyOperations(ref);
}

void efd::SolutionImplPass::visit(NDQOpReset::Ref ref) {
    applyOperations(ref);
}

void efd::SolutionImplPass::visit(NDQOpU::Ref ref) {
    applyOperations(ref);
}

void efd::SolutionImplPass::visit(NDQOpBarrier::Ref ref) {
    applyOperations(ref);
}

void efd::SolutionImplPass::visit(NDIfStmt::Ref ref) {
    applyOperations(ref->getQOp(), ref);
}

void efd::SolutionImplPass::visit(NDList::Ref ref) {
    for (uint32_t i = 0, e = ref->getChildNumber(); i < e; ++i) {
        ref->setChild(i, getMappedNode(ref->getChild(i)));
    }
}

void efd::SolutionImplPass::visit(NDQOpCX::Ref ref) {
    applyOperations(ref);
}

void efd::SolutionImplPass::visit(NDQOpGen::Ref ref) {
    applyOperations(ref);
}

// ------------------ StdSolutionQAllocator ----------------------
StdSolutionQAllocator::StdSolutionQAllocator(ArchGraph::sRef archGraph)
    : QbitAllocator(archGraph) {}

Mapping StdSolutionQAllocator::allocate(QModule::Ref qmod) {
    auto stdSolution = buildStdSolution(qmod);
    SolutionImplPass pass(stdSolution);
    PassCache::Run(mMod, &pass);
    return stdSolution.mInitial;
}
