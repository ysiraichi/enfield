#include "enfield/Transform/IntrinsicGateCostPass.h"
#include "enfield/Support/CommandLine.h"

using namespace efd;

efd::Opt<uint32_t> SwapCost
("-swap-cost", "Cost of using a swap function.", 7, false);
efd::Opt<uint32_t> RevCost
("-rev-cost", "Cost of using a reverse edge.", 4, false);
efd::Opt<uint32_t> LCXCost
("-lcx-cost", "Cost of using long cnot gate.", 10, false);

uint8_t IntrinsicGateCostPass::ID = 0;

bool IntrinsicGateCostPass::run(QModule::Ref qmod) {
    mData = 0;

    const uint8_t _SwapCost = SwapCost.getVal();
    const uint8_t _RevCost = RevCost.getVal();
    const uint8_t _LcxCost = LCXCost.getVal();

    for (auto i = qmod->stmt_begin(), e = qmod->stmt_end(); i != e; ++i) {
        auto node = i->get();
        auto qOpGen = dynCast<NDQOpGen>(node);

        if (qOpGen != nullptr && qOpGen->isIntrinsic()) {
            switch (qOpGen->getIntrinsicKind()) {
                case NDQOpGen::K_INTRINSIC_SWAP:
                    mData += _SwapCost;
                    break;

                case NDQOpGen::K_INTRINSIC_REV_CX:
                    mData += _RevCost;
                    break;

                case NDQOpGen::K_INTRINSIC_LCX:
                    mData += _LcxCost;
                    break;
            }
        }
    }

    return false;
}

IntrinsicGateCostPass::uRef IntrinsicGateCostPass::Create() {
    return uRef(new IntrinsicGateCostPass());
}
