#include "enfield/Transform/SemanticVerifierPass.h"
#include "enfield/Transform/CircuitGraphBuilderPass.h"
#include "enfield/Transform/XbitToNumberPass.h"
#include "enfield/Transform/FlattenPass.h"
#include "enfield/Transform/InlineAllPass.h"
#include "enfield/Transform/PassCache.h"
#include "enfield/Transform/Utils.h"
#include "enfield/Analysis/NodeVisitor.h"
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
            uint32_t mQubits;
            uint32_t mXbits;
            XbitToNumber& mXtoNSrc;
            XbitToNumber& mXtoNDst;
            CircuitGraph mCkt;
            Mapping mMap;

            std::map<CircuitNode*, uint32_t> mReached;
            std::vector<bool> mMarked;

            uint32_t getDstUId(const uint32_t srcUId, bool isQuantum = true);
            void updatedReachedCktNodes();
            bool advanceIntrinsicNodes();
            void advanceCktNodes(std::vector<uint32_t> xbitsToUpdate);
            void postprocessing(std::vector<uint32_t> xbitsToUpdate);
            void visitQOp(NDQOp::Ref qop, NDIfStmt::Ref ifstmt = nullptr);

        public:
            SemanticVerifierVisitor(XbitToNumber& xtonsrc,
                                    XbitToNumber& xtondst,
                                    CircuitGraph& ckt,
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
                                                 XbitToNumber& xtondst,
                                                 CircuitGraph& ckt,
                                                 Mapping initial) :
    mQubits(xtondst.getQSize()),
    mXbits(mQubits + xtondst.getCSize()),
    mXtoNSrc(xtonsrc),
    mXtoNDst(xtondst),
    mCkt(ckt),
    mMap(initial),
    mMarked(mXbits, false),
    mSuccess(true) {
    
    postprocessing({});
}

void SemanticVerifierVisitor::updatedReachedCktNodes() {
    for (uint32_t i = 0; i < mXbits; ++i) {
        auto cktNode = mCkt[i];

        if (cktNode && !mMarked[i]) {
            mMarked[i] = true;

            if (mReached.find(cktNode) == mReached.end())
                mReached[cktNode] = cktNode->qargsid.size() + cktNode->cargsid.size();
            --mReached[cktNode];
        }
    }
}

void SemanticVerifierVisitor::advanceCktNodes(std::vector<uint32_t> xbitsToUpdate) {
    for (uint32_t x : xbitsToUpdate) {
        mCkt[x] = mCkt[x]->child[x];
        mMarked[x] = false;
    }
}

bool SemanticVerifierVisitor::advanceIntrinsicNodes() {
    bool changed = false;

    std::set<std::pair<uint32_t, uint32_t>> swaps;

    for (uint32_t x = 0; x < mXbits; ++x) {
        if (mCkt[x] && !mReached[mCkt[x]]) {
            auto qopgen = dynCast<NDQOpGen>(mCkt[x]->node);

            if (qopgen && qopgen->isIntrinsic() &&
                    qopgen->getIntrinsicKind() == NDQOpGen::K_INTRINSIC_SWAP) {
                auto qargs = qopgen->getQArgs();
                assert(qargs->getChildNumber() == 2 && "Swaps only contains two nodes.");

                // auto assign = GenAssignment(mMap.size(), mMap);
                std::vector<uint32_t> assign(mMap.size(), 0);
                for (uint32_t i = 0, e = mMap.size(); i < e; ++i)
                    assign[mMap[i]] = i;

                uint32_t u = mXtoNDst.getQUId(qargs->getChild(0)->toString(false));
                uint32_t v = mXtoNDst.getQUId(qargs->getChild(1)->toString(false));

                swaps.insert(std::make_pair(assign[u], assign[v]));

                mCkt[x] = mCkt[x]->child[x];
                mMarked[x] = false;
                changed = true;
            }
        }
    }

    for (auto pair : swaps) {
        std::swap(mMap[pair.first], mMap[pair.second]);
    }

    return changed;
}

uint32_t SemanticVerifierVisitor::getDstUId(const uint32_t srcUId, bool isQuantum) {
    if (isQuantum) return mMap[srcUId];
    return mQubits + srcUId;
}

void SemanticVerifierVisitor::postprocessing(std::vector<uint32_t> xbitsToUpdate) {
    advanceCktNodes(xbitsToUpdate);

    bool changed;

    do {
        updatedReachedCktNodes();
        changed = advanceIntrinsicNodes();
    } while (changed);
}

void SemanticVerifierVisitor::visitQOp(NDQOp::Ref qop, NDIfStmt::Ref ifstmt) {
    // Checking all quantum arguments from the current node.
    std::vector<uint32_t> opQubits;
    std::vector<uint32_t> opCbits;

    auto qargs = qop->getQArgs();
    uint32_t qargsChildrem = qargs->getChildNumber();

    for (uint32_t i = 0; i < qargsChildrem; ++i) {
        auto qarg = qargs->getChild(i);
        uint32_t srcQUId = mXtoNSrc.getQUId(qarg->toString(false));
        opQubits.push_back(getDstUId(srcQUId));
    }

    if (ifstmt != nullptr) {
        for (auto cbit : mXtoNSrc.getRegUIds(ifstmt->getCondId()->getVal()))
            opCbits.push_back(getDstUId(cbit));
    }

    assert(!opQubits.empty() && "QOp without qubits?");

    // Checking all quantum arguments from the circuit node.
    std::vector<uint32_t> cktOpQubits;
    std::vector<uint32_t> cktOpCbits;

    auto cktCNode = mCkt[opQubits[0]];

    if (cktCNode == nullptr) { mSuccess = false; return; }

    auto cktNode = cktCNode->node;
    NDQOp::Ref cktQOp = nullptr;

    if (ifstmt != nullptr && ifstmt->getKind() == cktNode->getKind()) {
        auto ifstmt = static_cast<NDIfStmt*>(cktNode);
        cktQOp = ifstmt->getQOp();

        for (auto cbit : mXtoNDst.getRegUIds(ifstmt->getCondId()->getVal()))
            cktOpCbits.push_back(mQubits + cbit);

    } else if (ifstmt == nullptr) {
        cktQOp = static_cast<NDQOp*>(cktNode);
    } else {
        // The current node \p ref is a NDIfStmt and the circuit one is not.
        mSuccess = false;
        return;
    }

    auto cktQArgs = cktQOp->getQArgs();
    uint32_t cktQArgsChildrem = cktQArgs->getChildNumber();

    for (uint32_t i = 0; i < cktQArgsChildrem; ++i) {
        uint32_t qubit = mXtoNDst.getQUId(cktQArgs->getChild(i)->toString(false));
        cktOpQubits.push_back(qubit);
    }

    // All qubits involved in the current node must also be involved in the circuit node.
    for (uint32_t q : opQubits) {
        mSuccess = mSuccess &&
            std::find(cktOpQubits.begin(), cktOpQubits.end(), q) != cktOpQubits.end();
    }

    // All qubits and cbits have reached this node (and they are not null).
    auto firstCktNode = mCkt[cktOpQubits[0]];
    mSuccess = mSuccess && firstCktNode && !mReached[firstCktNode];

    // All used qubits have reached the same node (there is no instruction that is dependent
    // of others that is being executed before its dependencies) 
    for (uint32_t i = 1; i < cktQArgsChildrem; ++i) {
        mSuccess = mSuccess && mCkt[cktOpQubits[i]] == mCkt[cktOpQubits[i - 1]];
    }

    if (cktOpCbits.size() != opCbits.size())
        mSuccess = false;

    if (!mSuccess) return;

    // If this operation deals with more than one qubit, we assume it deals with exactly two
    // qubits, and that it is a CNOT gate.
    if (opQubits.size() > 1) {
        assert(opQubits.size() == 2 && "SemanticVerifier only handles CNOT gates.");

        // Both CNOTs and REV_CNOTS have the same semantic.
        // The only difference is in the way they are implemented. 
        SemanticCNOT srcCNOT { opQubits[0], opQubits[1] };
        SemanticCNOT transformedCNOT { cktOpQubits[0], cktOpQubits[1] };

        if (!IsCNOTGateCall(cktQOp)) {
            auto cktQOpGen = dynCast<NDQOpGen>(cktQOp);

            // Checking for intrinsics.
            if (cktQOpGen && cktQOpGen->isIntrinsic() &&
                    cktQOpGen->getIntrinsicKind() == NDQOpGen::K_INTRINSIC_LCX) {
                    transformedCNOT.u = cktOpQubits[0];
                    transformedCNOT.v = cktOpQubits[2];
            }
        }

        // Check if, semanticaly, CNOTs are applied to the same qubits in the same order.
        mSuccess = mSuccess && srcCNOT == transformedCNOT;

    } else {
        if (ifstmt != nullptr)
            mSuccess = mSuccess && firstCktNode->node->getKind() == ifstmt->getKind();
        else
            mSuccess = mSuccess && firstCktNode->node->getKind() == qop->getKind();

        // Checking all real arguments.
        auto refArgs = qop->getArgs();
        auto cktArgs = cktQOp->getArgs();
        mSuccess = mSuccess && refArgs->equals(cktArgs);
    }

    if (mSuccess) {
        std::vector<uint32_t> xbits = cktOpQubits;

        if (!cktOpCbits.empty()) {
            xbits.insert(xbits.begin(), cktOpCbits.begin(), cktOpCbits.end());
        }

        postprocessing(xbits);
    }
}

void SemanticVerifierVisitor::visit(NDQOpMeasure::Ref ref) {
    uint32_t srcQUId = mXtoNSrc.getQUId(ref->getQBit()->toString(false));
    uint32_t srcCUId = mXtoNSrc.getCUId(ref->getCBit()->toString(false));

    uint32_t qubitId = getDstUId(srcQUId);
    uint32_t cbitId = getDstUId(srcCUId, false);

    auto cktNode = mCkt[qubitId];
    mSuccess = mSuccess && cktNode && mCkt[qubitId] == mCkt[cbitId] && !mReached[cktNode]
        && cktNode->node->getKind() == ref->getKind();

    if (mSuccess) postprocessing({ qubitId, cbitId });
}

void SemanticVerifierVisitor::visit(NDQOpReset::Ref ref) {
    visitQOp(ref);
}

void SemanticVerifierVisitor::visit(NDQOpU::Ref ref) {
    visitQOp(ref);
}

void SemanticVerifierVisitor::visit(NDQOpCX::Ref ref) {
    visitQOp(ref);
}

void SemanticVerifierVisitor::visit(NDQOpBarrier::Ref ref) {
    visitQOp(ref);
}

void SemanticVerifierVisitor::visit(NDQOpGen::Ref ref) {
    visitQOp(ref);
}

void SemanticVerifierVisitor::visit(NDIfStmt::Ref ref) {
    visitQOp(ref->getQOp(), ref);
}

SemanticVerifierPass::SemanticVerifierPass(QModule::uRef src, Mapping initial)
    : mSrc(std::move(src)), mInitial(initial) {
    mData = false;
}

bool SemanticVerifierPass::run(QModule* dst) {
    PassCache::Run<FlattenPass>(mSrc.get());
    PassCache::Run<InlineAllPass>(mSrc.get());

    auto cktpass = PassCache::Get<CircuitGraphBuilderPass>(dst);
    auto& ckt = cktpass->getData();

    auto xtonpassSrc = PassCache::Get<XbitToNumberWrapperPass>(mSrc.get());
    auto xtonpassDst = PassCache::Get<XbitToNumberWrapperPass>(dst);

    mData = true;
    SemanticVerifierVisitor visitor(xtonpassSrc->getData(), xtonpassDst->getData(), ckt, mInitial);
    for (auto it = mSrc->stmt_begin(), end = mSrc->stmt_end();
            it != end && mData; ++it) {
        (*it)->apply(&visitor);
        mData = mData && visitor.mSuccess;
    }

    return false;
}

SemanticVerifierPass::uRef SemanticVerifierPass::Create(QModule::uRef src, Mapping initial) {
    return uRef(new SemanticVerifierPass(std::move(src), initial));
}
