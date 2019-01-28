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
            InverseMap mInverseMap;

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
            ResultMsg& mResult;

            SemanticVerifierVisitor(ResultMsg& result,
                                    XbitToNumber& xtonsrc,
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
    };
}

SemanticVerifierVisitor::SemanticVerifierVisitor(ResultMsg& result,
                                                 XbitToNumber& xtonsrc,
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
    mResult(result) {
    

    mInverseMap.assign(mQubitsTgt, 0);
    for (uint32_t i = 0; i < mQubitsTgt; ++i) {
        mInverseMap[mMap[i]] = i;
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
    if (tgtUId < mQubitsTgt) return mInverseMap[tgtUId];
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

    Node::Ref mainNode = tgtQOp;
    if (tgtIfStmt != nullptr) mainNode = tgtIfStmt;

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

    if (!srcCNode->isGateNode()) {
        mResult = ResultMsg::Error(
                "Original program has reached the end while processing `" +
                mainNode->toString(false) + "` of the target program.");
        return;
    }

    auto srcNode = srcCNode->node();
    NDQOp::Ref srcQOp = nullptr;

    if (tgtIfStmt != nullptr) {
        if (tgtIfStmt->getKind() != srcNode->getKind()) {
            mResult = ResultMsg::Error(
                    "Source instruction `" + srcNode->toString(false) + "` " +
                    "is not an NDIfStmt: " + mainNode->toString(false) + ".");
            return;
        }

        auto srcIfStmt = static_cast<NDIfStmt*>(srcNode);
        srcQOp = srcIfStmt->getQOp();

        for (auto cbit : mXtoNSrc.getRegUIds(srcIfStmt->getCondId()->getVal()))
            srcOpCbits.push_back(getTgtUId(getRealSrcCUId(cbit)));
    } else {
        srcQOp = dynCast<NDQOp>(srcNode);
    }

    if (srcQOp == nullptr) {
        // Either the current node \em tgtQOp is a NDIfStmt and the circuit one is not or
        // the other way around.
        mResult = ResultMsg::Error(
                "Expected `" + srcNode->toString(false) +
                "` from source program. Got `" + mainNode->toString(false) +
                "` from target program.");
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
        if (std::find(tgtOpQubits.begin(), tgtOpQubits.end(), q) == tgtOpQubits.end()) {
            mResult = ResultMsg::Error(
                    "Qubit `" + std::to_string(getSrcUId(q)) + " => " +
                    std::to_string(q) + "` not being used in " +
                    "target program (" + mainNode->toString(false) +
                    "). Expected: `" + srcNode->toString(false) + "`.");
            return;
        }
    }

    // All qubits and cbits have reached this node (and they are not null).
    auto firstSrcCNode = mIt[getSrcUId(srcOpQubits[0])];
    auto firstSrcNode = firstSrcCNode->node();

    if (!firstSrcCNode->isGateNode()) {
        mResult = ResultMsg::Error(
                "Qubit `" + std::to_string(getSrcUId(srcOpQubits[0])) + " => " +
                std::to_string(srcOpQubits[0]) + "` has already reached its end " +
                "while processing: " + mainNode->toString(false) + ".");
        return;
    }

    if (mReached[firstSrcNode]) {
        mResult = ResultMsg::Error(
                "Node `" + firstSrcNode->toString(false) + "` still lacks some " +
                "dependencies.");
        return;
    }

    // All used qubits have reached the same node (there is no instruction that is dependent
    // of others that is being executed before its dependencies) 
    // for (uint32_t i = 1; i < srcQArgsChildrem; ++i) {
    //     mSuccess = mSuccess && mIt[getSrcUId(srcOpQubits[i])]->node() == firstSrcNode;
    // }

    if (srcOpCbits.size() != tgtOpCbits.size()) {
        mResult = ResultMsg::Error(
                "Nodes `" + srcNode->toString(false) + "` (source) and " +
                "`" + mainNode->toString(false) + "` (target) have different " +
                "number of concrete bits.");
        return;
    }

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
            if (srcCNOT != tgtCNOT) {
                mResult = ResultMsg::Error(
                        "CNOT error. Expected (" + std::to_string(srcCNOT.u) + ", " +
                        std::to_string(srcCNOT.v) + "). Got (" +
                        std::to_string(tgtCNOT.u) + ", " +
                        std::to_string(tgtCNOT.v) + ").");
                return;
            }

        } else if (!instanceOf<NDQOpBarrier>(srcQOp)) {
            EfdAbortIf(true,
                       "Node is neither CNOT nor Barrier. Actual: `"
                       << srcQOp->toString(false) << "`.");
        }

    } else {
        if (firstSrcCNode->node()->getKind() != mainNode->getKind()) {
            mResult = ResultMsg::Error(
                    "Wrong kind between source (" + srcNode->toString(false) + ") " +
                    "and target (" + mainNode->toString(false) + ").");
            return;
        }

        // Checking all real arguments.
        auto tgtArgs = tgtQOp->getArgs();
        auto srcArgs = srcQOp->getArgs();

        if (!tgtArgs->equals(srcArgs)) {
            mResult = ResultMsg::Error(
                    "Real arguments do not match: `" + srcArgs->toString(false) +
                    "`(source) and `" + tgtArgs->toString(false) + "`(target).");
            return;
        }
    }

    std::vector<uint32_t> all;
    std::vector<uint32_t> toBeAdvanced;

    all.insert(all.begin(), srcOpQubits.begin(), srcOpQubits.end());
    all.insert(all.begin(), srcOpCbits.begin(), srcOpCbits.end());

    for (auto q : all) {
        toBeAdvanced.push_back(getSrcUId(q));
    }

    postprocessing(toBeAdvanced);
}

void SemanticVerifierVisitor::visit(NDQOpMeasure::Ref ref) {
    uint32_t tgtQUId = mXtoNTgt.getQUId(ref->getQBit()->toString(false));
    uint32_t tgtCUId = getRealTgtCUId(mXtoNTgt.getCUId(ref->getCBit()->toString(false)));

    auto srcCNode = mIt[getSrcUId(tgtQUId)];

    if (!srcCNode->isGateNode()) {
        mResult = ResultMsg::Error(
                "Source has reached the end while processing: " +
                ref->toString(false) + ".");
        return;
    }

    auto srcNode = dynCast<NDQOpMeasure>(srcCNode->node());

    if (srcNode != nullptr) {
        uint32_t srcQUId = mXtoNSrc.getQUId(srcNode->getQBit()->toString(false));
        uint32_t srcCUId = getRealSrcCUId(mXtoNSrc.getCUId(srcNode->getCBit()->toString(false)));

        if (tgtQUId != getTgtUId(srcQUId) || tgtCUId != getTgtUId(srcCUId)) {
            mResult = ResultMsg::Error(
                    "Measure instruction does not match: `" +
                    srcNode->toString(false) + "`(source) and `" +
                    ref->toString(false) + "`(target).");
            return;
        }

        if (mReached[srcNode] && mIt[srcQUId]->node() != mIt[srcCUId]->node()) {
            mResult = ResultMsg::Error(
                    "Node `" + ref->toString(false) + "` still lacks some " +
                    "dependencies.");
            return;
        }

        postprocessing({ srcQUId, srcCUId });
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

        uint32_t a = mInverseMap[u];
        uint32_t b = mInverseMap[v];

        std::swap(mMap[a], mMap[b]);
        std::swap(mInverseMap[u], mInverseMap[v]);
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
    mData = ResultMsg::Success();
}

bool SemanticVerifierPass::run(QModule* tgt) {
    PassCache::Run<FlattenPass>(mSrc.get());

    auto inlinePass = InlineAllPass::Create(mBasis);
    PassCache::Run(mSrc.get(), inlinePass.get());

    auto cktpass = PassCache::Get<CircuitGraphBuilderPass>(mSrc.get());
    auto& ckt = cktpass->getData();

    auto xtonpassSrc = PassCache::Get<XbitToNumberWrapperPass>(mSrc.get());
    auto xtonpassTgt = PassCache::Get<XbitToNumberWrapperPass>(tgt);

    mData = ResultMsg::Success();
    auto it = ckt.build_iterator();
    SemanticVerifierVisitor visitor(mData,
                                    xtonpassSrc->getData(),
                                    xtonpassTgt->getData(),
                                    it,
                                    mInitial);

    for (auto it = tgt->stmt_begin(), end = tgt->stmt_end(); it != end; ++it) {
        (*it)->apply(&visitor);
        if (mData.isError()) return false;
    }

    for (uint32_t i = 0, e = ckt.size(); i < e; ++i) {
        if (!it[i]->isOutputNode()) {
            mData = ResultMsg::Error(
                    "Stopped at `" + it.get(i)->toString(false) +
                    "` on qubit `" + std::to_string(i) + "`.");
            break;
        }
    }

    return false;
}

void SemanticVerifierPass::setInlineAll(std::vector<std::string> basis) {
    mBasis = basis;
}

SemanticVerifierPass::uRef SemanticVerifierPass::Create(QModule::uRef src, Mapping initial) {
    return uRef(new SemanticVerifierPass(std::move(src), initial));
}
