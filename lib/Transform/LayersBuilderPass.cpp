#include "enfield/Transform/LayersBuilderPass.h"
#include "enfield/Transform/XbitToNumberPass.h"
#include "enfield/Transform/QModule.h"
#include "enfield/Transform/PassCache.h"
#include "enfield/Analysis/NodeVisitor.h"

#include <algorithm>

using namespace efd;

uint8_t LayersBuilderPass::ID = 0;

namespace efd {
    class UsedBitsVisitor : public NodeVisitor {
        public:
            QModule::Ref mMod;
            XbitToNumber& mXton;
            std::vector<uint32_t> mBits;

            UsedBitsVisitor(QModule::Ref qmod, XbitToNumber& xton);

            void visitQOp(NDQOp::Ref ref);

            void visit(NDQOpMeasure::Ref ref) override;
            void visit(NDQOpReset::Ref ref) override;
            void visit(NDQOpU::Ref ref) override;
            void visit(NDQOpCX::Ref ref) override;
            void visit(NDQOpBarrier::Ref ref) override;
            void visit(NDQOpGen::Ref ref) override;
            void visit(NDIfStmt::Ref ref) override;
    };
}

UsedBitsVisitor::UsedBitsVisitor(QModule::Ref qmod, XbitToNumber& xton) : mMod(qmod), mXton(xton) {}

void UsedBitsVisitor::visitQOp(NDQOp::Ref ref) {
    mBits.clear();
    for (auto& qarg : *(ref->getQArgs())) {
        uint32_t qid = mXton.getQUId(qarg->toString(false));
        mBits.push_back(qid);
    }
}

void UsedBitsVisitor::visit(NDQOpMeasure::Ref ref) {
    visitQOp(ref);
    mBits.push_back(mXton.getQSize() + mXton.getCUId(ref->getCBit()->toString(false)));
}

void UsedBitsVisitor::visit(NDQOpReset::Ref ref) {
    visitQOp(ref);
}

void UsedBitsVisitor::visit(NDQOpU::Ref ref) {
    visitQOp(ref);
}

void UsedBitsVisitor::visit(NDQOpCX::Ref ref) {
    visitQOp(ref);
}

void UsedBitsVisitor::visit(NDQOpBarrier::Ref ref) {
    visitQOp(ref);
}

void UsedBitsVisitor::visit(NDQOpGen::Ref ref) {
    visitQOp(ref);
}

void UsedBitsVisitor::visit(NDIfStmt::Ref ref) {
    visitQOp(ref->getQOp());

    auto cregname = ref->getCondId()->getVal();
    auto cregDecl = dynCast<NDRegDecl>(mMod->getQVar(cregname));
    auto size = cregDecl->getSize()->getVal().mV;

    for (uint32_t i = 0; i < size; ++i) {
        std::string sid = cregname + "[" + std::to_string(i) + "]";
        uint32_t cid = mXton.getCUId(sid);
        mBits.push_back(mXton.getQSize() + cid);
    }
}

bool LayersBuilderPass::run(QModule* qmod) {
    auto xtonPass = PassCache::Get<XbitToNumberWrapperPass>(qmod);
    auto& xton = xtonPass->getData();

    uint32_t qubits = xton.getQSize();
    uint32_t cbits = xton.getCSize();

    UsedBitsVisitor ubVisitor(qmod, xton);
    std::vector<int32_t> layerNum(qubits + cbits, -1);

    for (auto it = qmod->stmt_begin(), end = qmod->stmt_end(); it != end; ++it) {
        auto node = it->get();

        int32_t maxLayer = 0;
        (*it)->apply(&ubVisitor);

        auto bits = ubVisitor.mBits;

        for (uint32_t i : bits) {
            maxLayer = std::max(maxLayer, layerNum[i] + 1);
        }

        for (uint32_t i : bits) {
            layerNum[i] = maxLayer;
        }

        if (mData.size() <= (uint32_t) maxLayer) {
            mData.push_back(Layer());
        }

        mData[maxLayer].push_back(node);
    }

    return false;
}

LayersBuilderPass::uRef LayersBuilderPass::Create() {
    return uRef(new LayersBuilderPass());
}
