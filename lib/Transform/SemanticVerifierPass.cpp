#include "enfield/Transform/SemanticVerifierPass.h"
#include "enfield/Transform/CircuitGraphBuilderPass.h"
#include "enfield/Transform/FlattenPass.h"
#include "enfield/Transform/PassCache.h"
#include "enfield/Analysis/NodeVisitor.h"

#include <unordered_map>
#include <vector>

using namespace efd;

static void populateSet(set<uint32_t>&, uint32_t);

namespace efd {
    class SemanticVerifierVisitor : public NodeVisitor {
        private:
            typedef set<uint32_t> IdSet;
            typedef vector<IdSet> Map;

            XbitToNumber& mXtoN;
            CircuitGraph& mG;
            CircuitGraph mLast;
            Map mMap;

        public:
            SemanticVerifierVisitor(XbitToNumber& xton, CircuitGraph& graph);

            void visit(NDQOpMeasure::Ref ref);
            void visit(NDQOpReset::Ref ref);
            void visit(NDQOpU::Ref ref);
            void visit(NDQOpCX::Ref ref);
            void visit(NDQOpBarrier::Ref ref);
            void visit(NDQOpGen::Ref ref);
            void visit(NDIfStmt::Ref ref);
    };
}

SemanticVerifierVisitor::SemanticVerifierVisitor(XbitToNumber& xton, CircuitGraph& graph)
    : mXtoN(xton), mG(graph), mLast(graph), mMap(xton.getQSize(), IdSet()) {}

uint32_t SemanticVerifierVisitor::initQubit(Node::Ref node) {
    auto qubitStr = node->toString(false);
    uint32_t qubitId = mXtoN.getQUId(qubitStr);

    if (mMap[qubitId].empty()) {
        populateSet(mMap[qubitId], mXtoN.getQSize());
    }

    return qubitId;
}

uint32_t SemanticVerifierVisitor::initCbit(Node::Ref node) {
    auto cbitStr = node->toString(false);
    uint32_t cbitId = mXtoN.getQUId(cbitStr);

    if (mMap[cbitId].empty()) {
        populateSet(mMap[cbitId], mXtoN.getCSize());
    }

    return cbitId;
}

void SemanticVerifierVisitor::visit(NDQOpMeasure::Ref ref) {
    uint32_t qubitId = initQubit(ref->getQbit());
    uint32_t cbitId = initCbit(ref->getCbit());

    vector<uint32_t> qNotCand;
    set<uint32_t> cCand;

    for (uint32_t i : mMap[qubitId]) {
        if (mLast[i] == nullptr || !instanceOf<NDQOpMeasure>(mLast[i]->node)) {
            notCand.push_back(i);
        } else {
            NDQOpMeasure::Ref measure = (NDQOpMeasure::Ref) mLast[i]->node;
            uint32_t cbitIdCand = mXtoN.getCUId(measure->getCBit()->toString(false));

            if (mMap[cbitId].find(cbitIdCand) != mMap[cbitId].end()) {
                cCand.insert();
            }
        }
    }

    for (uint32_t i : notCand)
        mMap[qubitId].erase(i);
    mMap[cbitId] = cCand;

    if (mMap[qubitId].empty() || mMap[cbitId].empty()) {
        assert(false && "SemanticVerifier fail at NDQOpMeasure.");
    } else {
    }
}

void SemanticVerifierVisitor::visit(NDQOpReset::Ref ref) {
    uint32_t qubitId = initQubit(ref->getQArg());
    vector<uint32_t> qNotCand;

    for (uint32_t i : mMap[qubitId]) {
        if (mLast[i] == nullptr || !instanceOf<NDQOpMeasure>(mLast[i]->node)) {
            notCand.push_back(i);
        } else {
            NDQOpMeasure::Ref measure = (NDQOpMeasure::Ref) mLast[i]->node;
            uint32_t cbitIdCand = mXtoN.getCUId(measure->getCBit()->toString(false));

            if (mMap[cbitId].find(cbitIdCand) != mMap[cbitId].end()) {
                cCand.insert();
            }
        }
    }

    for (uint32_t i : notCand)
        mMap[qubitId].erase(i);
}

void SemanticVerifierVisitor::visit(NDQOpU::Ref ref) {
}

void SemanticVerifierVisitor::visit(NDQOpCX::Ref ref) {
}

void SemanticVerifierVisitor::visit(NDQOpBarrier::Ref ref) {
}

void SemanticVerifierVisitor::visit(NDQOpGen::Ref ref) {
}

void SemanticVerifierVisitor::visit(NDIfStmt::Ref ref) {
}

SemanticVerifierPass::SemanticVerifierPass(QModule::uRef src) : mSuccess(false), mSrc(src) {}

bool SemanticVerifierPass::run(QModule* dst) {
    PassCache::Run<FlattenPass>(dst);
    auto cktpass = PassCache::Get<CircuitGraphBuilderPass>(dst);

    auto& graph = cktpass->getData();

    SemanticVerifierVisitor visitor(graph);
    for (auto it = mSrc->stmt_begin(), end = mSrc->stmt_end(); it != end; ++it) {
    }

    return false;
}

static void populateSet(set<uint32_t>& s, uint32_t n) {
    for (uint32_t i = 0; i < n; ++i)
        s.insert(i);
}
