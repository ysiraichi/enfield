#include "enfield/Transform/SemanticVerifierPass.h"
#include "enfield/Transform/CircuitGraphBuilderPass.h"
#include "enfield/Transform/XbitToNumberPass.h"
#include "enfield/Transform/FlattenPass.h"
#include "enfield/Transform/InlineAllPass.h"
#include "enfield/Transform/PassCache.h"
#include "enfield/Transform/Utils.h"
#include "enfield/Analysis/NodeVisitor.h"
#include "enfield/Support/Defs.h"
#include "enfield/Support/RTTI.h"

#include <unordered_map>
#include <vector>
#include <cassert>
#include <algorithm>

using namespace efd;

namespace {
    struct SemanticCNOT {
        uint32_t u, v;

        bool operator==(const SemanticCNOT& rhs) {
            return u == rhs.u && v == rhs.v;
        }

        bool operator!=(const SemanticCNOT& rhs) {
            return !(*this == rhs);
        }
    };
}

namespace efd {
    class SemanticVerifierVisitor : public NodeVisitor {
        private:
            uint32_t mQubitsSrc;
            uint32_t mQubitsTgt;
            uint32_t mXbitsSrc;
            XbitToNumber& mXtoNSrc;
            XbitToNumber& mXtoNTgt;
            CircuitGraph::Iterator& mIt;
            Mapping mMap;
            Assign mAssign;

            std::map<Node::Ref, uint32_t> mReached;
            std::vector<bool> mMarked;

            inline uint32_t getTgtUId(uint32_t srcUId);
            inline uint32_t getSrcUId(uint32_t tgtUId);
            inline uint32_t getRealTgtCUId(uint32_t baseUId);
            inline uint32_t getRealSrcCUId(uint32_t baseUId);

            void updatedReachedCktNodes();
            void advanceCktNodes(std::vector<uint32_t> xbitsToUpdate);
            void postprocessing(std::vector<uint32_t> xbitsToUpdate);
            void visitNDQOp(NDQOp::Ref qop, NDIfStmt::Ref ifstmt = nullptr);

        public:
            SemanticVerifierVisitor(XbitToNumber& xtonsrc,
                                    XbitToNumber& xtontgt,
                                    CircuitGraph::Iterator& it,
                                    Mapping initial);

            void visit(NDQOpMeasure::Ref ref) override;
            void visit(NDQOpReset::Ref ref) override;
            void visit(NDQOpU::Ref ref) override;
            void visit(NDQOpCX::Ref ref) override;
            void visit(NDQOpBarrier::Ref ref) override;
            void visit(NDQOpGen::Ref ref) override;
            void visit(NDIfStmt::Ref ref) override;

            bool mSuccess;
    };
}

SemanticVerifierVisitor::SemanticVerifierVisitor(XbitToNumber& xtonsrc,
                                                 XbitToNumber& xtontgt,
                                                 CircuitGraph::Iterator& iterator,
                                                 Mapping initial) :
    mQubitsSrc(xtonsrc.getQSize()),
    mQubitsTgt(xtontgt.getQSize()),
    mXbitsSrc(xtonsrc.getQSize() + xtonsrc.getCSize()),
    mXtoNSrc(xtonsrc),
    mXtoNTgt(xtontgt),
    mIt(iterator),
    mMap(initial),
    mMarked(mXbitsSrc, false),
    mSuccess(true) {
    

    mAssign.assign(mQubitsTgt, 0);
    for (uint32_t i = 0; i < mQubitsTgt; ++i) {
        mAssign[mMap[i]] = i;
    }

    for (uint32_t i = 0; i < mXbitsSrc; ++i) {
        mIt.next(i);
    }

    postprocessing({});
}

uint32_t SemanticVerifierVisitor::getRealTgtCUId(uint32_t baseUId) {
    return mQubitsTgt + baseUId;
}

uint32_t SemanticVerifierVisitor::getRealSrcCUId(uint32_t baseUId) {
    return mQubitsSrc + baseUId;
}

uint32_t SemanticVerifierVisitor::getTgtUId(uint32_t srcUId) {
    if (srcUId < mQubitsSrc) return mMap[srcUId];
    return (srcUId - mQubitsSrc) + mQubitsTgt;
}

uint32_t SemanticVerifierVisitor::getSrcUId(uint32_t tgtUId) {
    if (tgtUId < mQubitsTgt) return mAssign[tgtUId];
    return (tgtUId - mQubitsTgt) + mQubitsSrc;
}

void SemanticVerifierVisitor::updatedReachedCktNodes() {
    for (uint32_t i = 0; i < mXbitsSrc; ++i) {
        auto circuitNode = mIt[i];
        auto node = circuitNode->node();

        if (circuitNode->isGateNode() && !mMarked[i]) {
            mMarked[i] = true;

            if (mReached.find(node) == mReached.end())
                mReached[node] = circuitNode->numberOfXbits();
            --mReached[node];
        }
    }
}

void SemanticVerifierVisitor::advanceCktNodes(std::vector<uint32_t> xbitsToUpdate) {
    for (uint32_t x : xbitsToUpdate) {
        mIt.next(x);
        mMarked[x] = false;
    }
}

void SemanticVerifierVisitor::postprocessing(std::vector<uint32_t> xbitsToUpdate) {
    advanceCktNodes(xbitsToUpdate);
    updatedReachedCktNodes();
}

void SemanticVerifierVisitor::visitNDQOp(NDQOp::Ref tgtQOp, NDIfStmt::Ref tgtIfStmt) {
    // Checking all quantum arguments from the current node.
    std::vector<uint32_t> tgtOpQubits;
    std::vector<uint32_t> tgtOpCbits;

    auto tgtQArgs = tgtQOp->getQArgs();
    uint32_t tgtQArgsChildrem = tgtQArgs->getChildNumber();

    for (uint32_t i = 0; i < tgtQArgsChildrem; ++i) {
        auto qarg = tgtQArgs->getChild(i);
        tgtOpQubits.push_back(mXtoNTgt.getQUId(qarg->toString(false)));
    }

    if (tgtIfStmt != nullptr) {
        for (auto cbit : mXtoNTgt.getRegUIds(tgtIfStmt->getCondId()->getVal()))
            tgtOpCbits.push_back(getRealTgtCUId(cbit));
    }

    // Checking all quantum arguments from the circuit node.
    std::vector<uint32_t> srcOpQubits;
    std::vector<uint32_t> srcOpCbits;

    auto srcCNode = mIt[getSrcUId(tgtOpQubits[0])];

    if (!srcCNode->isGateNode()) { mSuccess = false; return; }

    auto srcNode = srcCNode->node();
    NDQOp::Ref srcQOp = nullptr;

    if (tgtIfStmt != nullptr && tgtIfStmt->getKind() == srcNode->getKind()) {
        auto srcIfStmt = static_cast<NDIfStmt*>(srcNode);
        srcQOp = srcIfStmt->getQOp();

        for (auto cbit : mXtoNSrc.getRegUIds(srcIfStmt->getCondId()->getVal()))
            srcOpCbits.push_back(getTgtUId(getRealSrcCUId(cbit)));

    } else if (tgtIfStmt == nullptr) {
        srcQOp = dynCast<NDQOp>(srcNode);
    }

    if (srcQOp == nullptr) {
        // Either the current node \em tgtQOp is a NDIfStmt and the circuit one is not or
        // the other way around.
        mSuccess = false;
        return;
    }

    auto srcQArgs = srcQOp->getQArgs();
    uint32_t srcQArgsChildrem = srcQArgs->getChildNumber();

    for (uint32_t i = 0; i < srcQArgsChildrem; ++i) {
        uint32_t qubit = mXtoNSrc.getQUId(srcQArgs->getChild(i)->toString(false));
        srcOpQubits.push_back(getTgtUId(qubit));
    }

    // All qubits involved in the current circuit node must also be involved in the current node.
    for (uint32_t q : srcOpQubits) {
        mSuccess = mSuccess &&
            std::find(tgtOpQubits.begin(), tgtOpQubits.end(), q) != tgtOpQubits.end();
    }

    // All qubits and cbits have reached this node (and they are not null).
    auto firstSrcCNode = mIt[getSrcUId(srcOpQubits[0])];
    auto firstSrcNode = firstSrcCNode->node();
    mSuccess = mSuccess && firstSrcCNode->isGateNode() && !mReached[firstSrcNode];

    // All used qubits have reached the same node (there is no instruction that is dependent
    // of others that is being executed before its dependencies) 
    for (uint32_t i = 1; i < srcQArgsChildrem; ++i) {
        mSuccess = mSuccess && mIt[getSrcUId(srcOpQubits[i])]->node() == firstSrcNode;
    }

    if (srcOpCbits.size() != tgtOpCbits.size())
        mSuccess = false;

    if (!mSuccess) return;

    // If this operation deals with more than one qubit, we assume it deals with exactly two
    // qubits, and that it is a CNOT gate.
    if (srcOpQubits.size() > 1) {
        if (IsCNOTGateCall(srcQOp)) {
            // Both CNOTs and REV_CNOTS have the same semantic.
            // The only difference is in the way they are implemented. 
            SemanticCNOT srcCNOT { srcOpQubits[0], srcOpQubits[1] };
            SemanticCNOT tgtCNOT { tgtOpQubits[0], tgtOpQubits[1] };

            if (!IsCNOTGateCall(tgtQOp)) {
                auto tgtQOpGen = dynCast<NDQOpGen>(tgtQOp);

                // Checking for intrinsics.
                if (tgtQOpGen && tgtQOpGen->isIntrinsic() &&
                        tgtQOpGen->getIntrinsicKind() == NDQOpGen::K_INTRINSIC_LCX) {
                        tgtCNOT.u = tgtOpQubits[0];
                        tgtCNOT.v = tgtOpQubits[2];
                }
            }

            // Check if, semanticaly, CNOTs are applied to the same qubits in the same order.
            mSuccess = mSuccess && srcCNOT == tgtCNOT;

        } else if (instanceOf<NDQOpBarrier>(srcQOp)) {

            auto src = dynCast<NDQOpBarrier>(srcQOp);
            auto tgt = dynCast<NDQOpBarrier>(tgtQOp);

            mSuccess = src->getQArgs()->getChildNumber() == tgt->getQArgs()->getChildNumber();

        } else {
            assert(false && "Node is neither CNOT nor Barrier.");
        }

    } else {
        if (tgtIfStmt != nullptr)
            mSuccess = mSuccess && firstSrcCNode->node()->getKind() == tgtIfStmt->getKind();
        else
            mSuccess = mSuccess && firstSrcCNode->node()->getKind() == tgtQOp->getKind();

        // Checking all real arguments.
        auto tgtArgs = tgtQOp->getArgs();
        auto srcArgs = srcQOp->getArgs();
        mSuccess = mSuccess && tgtArgs->equals(srcArgs);
    }

    if (mSuccess) {
        std::vector<uint32_t> all;
        std::vector<uint32_t> toBeAdvanced;

        all.insert(all.begin(), srcOpQubits.begin(), srcOpQubits.end());
        all.insert(all.begin(), srcOpCbits.begin(), srcOpCbits.end());

        for (auto q : all) {
            toBeAdvanced.push_back(getSrcUId(q));
        }

        postprocessing(toBeAdvanced);
    }
}

void SemanticVerifierVisitor::visit(NDQOpMeasure::Ref ref) {
    uint32_t tgtQUId = mXtoNTgt.getQUId(ref->getQBit()->toString(false));
    uint32_t tgtCUId = getRealTgtCUId(mXtoNTgt.getCUId(ref->getCBit()->toString(false)));

    auto srcCNode = mIt[getSrcUId(tgtQUId)];
    auto srcNode = dynCast<NDQOpMeasure>(srcCNode->node());

    if (srcNode != nullptr) {
        uint32_t srcQUId = mXtoNSrc.getQUId(srcNode->getQBit()->toString(false));
        uint32_t srcCUId = getRealSrcCUId(mXtoNSrc.getCUId(srcNode->getCBit()->toString(false)));

        mSuccess = mSuccess && tgtQUId == getTgtUId(srcQUId);
        mSuccess = mSuccess && tgtCUId == getTgtUId(srcCUId);

        mSuccess = mSuccess && srcCNode->isGateNode();
        mSuccess = mSuccess && !mReached[srcNode];
        mSuccess = mSuccess && mIt[srcQUId]->node() == mIt[srcCUId]->node();
        mSuccess = mSuccess && srcNode->getKind() == ref->getKind();

        if (mSuccess) postprocessing({ srcQUId, srcCUId });
    }
}

void SemanticVerifierVisitor::visit(NDQOpReset::Ref ref) {
    visitNDQOp(ref);
}

void SemanticVerifierVisitor::visit(NDQOpU::Ref ref) {
    visitNDQOp(ref);
}

void SemanticVerifierVisitor::visit(NDQOpCX::Ref ref) {
    visitNDQOp(ref);
}

void SemanticVerifierVisitor::visit(NDQOpBarrier::Ref ref) {
    visitNDQOp(ref);
}

void SemanticVerifierVisitor::visit(NDQOpGen::Ref ref) {
    if (ref->isIntrinsic() && ref->getIntrinsicKind() == NDQOpGen::K_INTRINSIC_SWAP) {
        auto qargs = ref->getQArgs();

        uint32_t u = mXtoNTgt.getQUId(qargs->getChild(0)->toString(false));
        uint32_t v = mXtoNTgt.getQUId(qargs->getChild(1)->toString(false));

        uint32_t a = mAssign[u];
        uint32_t b = mAssign[v];

        std::swap(mMap[a], mMap[b]);
        std::swap(mAssign[u], mAssign[v]);
    } else {
        visitNDQOp(ref);
    }
}

void SemanticVerifierVisitor::visit(NDIfStmt::Ref ref) {
    // NOTE: if, one day, we have 'swap' nodes inside if statements, we
    // should call 'ref->getQOp()->apply()' here.
    visitNDQOp(ref->getQOp(), ref);
}

SemanticVerifierPass::SemanticVerifierPass(QModule::uRef src, Mapping initial)
    : mSrc(std::move(src)), mInitial(initial) {
    mData = false;
}

bool SemanticVerifierPass::run(QModule* tgt) {
    PassCache::Run<FlattenPass>(mSrc.get());

    auto inlinePass = InlineAllPass::Create(mBasis);
    PassCache::Run(mSrc.get(), inlinePass.get());

    auto cktpass = PassCache::Get<CircuitGraphBuilderPass>(mSrc.get());
    auto& ckt = cktpass->getData();

    auto xtonpassSrc = PassCache::Get<XbitToNumberWrapperPass>(mSrc.get());
    auto xtonpassTgt = PassCache::Get<XbitToNumberWrapperPass>(tgt);

    mData = true;
    auto it = ckt.build_iterator();
    SemanticVerifierVisitor visitor(xtonpassSrc->getData(),
                                    xtonpassTgt->getData(),
                                    it,
                                    mInitial);
    for (auto it = tgt->stmt_begin(), end = tgt->stmt_end();
            it != end && mData; ++it) {
        (*it)->apply(&visitor);
        mData = mData && visitor.mSuccess;
    }

    for (uint32_t i = 0, e = ckt.size(); i < e; ++i) {
        if (!it[i]->isOutputNode()) { mData = false; break; }
    }

    return false;
}

void SemanticVerifierPass::setInlineAll(std::vector<std::string> basis) {
    mBasis = basis;
}

SemanticVerifierPass::uRef SemanticVerifierPass::Create(QModule::uRef src, Mapping initial) {
    return uRef(new SemanticVerifierPass(std::move(src), initial));
}
