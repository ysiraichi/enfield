#include "enfield/Transform/ErrorRateCalculationPass.h"
#include "enfield/Transform/Utils.h"

using namespace efd;

uint8_t efd::ErrorRateCalculationPass::ID = 0;

ErrorRateCalculationPass::ErrorRateCalculationPass(ArchGraph::sRef arch) : mArchGraph(arch) {}

bool ErrorRateCalculationPass::run(QModule::Ref qmod) {
    EfdAbortIf(mArchGraph.get() == nullptr,
               "mArchGraph not defined for `ErrorRateCalculationPass`.");

    double result = 1;

    for (auto it = qmod->stmt_begin(), end = qmod->stmt_end(); it != end; ++it) {
        auto qopNode = GetStatementPair(it->get()).second;

        if (IsCNOTGateCall(qopNode)) {
            auto qargs = qopNode->getQArgs();
            uint32_t u = mArchGraph->getUId(qargs->getChild(0)->toString(false));
            uint32_t v = mArchGraph->getUId(qargs->getChild(1)->toString(false));
            result = result * mArchGraph->getW(u, v);
        }
    }

    mData = 1 - result;

    return false;
}

ErrorRateCalculationPass::uRef ErrorRateCalculationPass::Create(ArchGraph::sRef arch) {
    return uRef(new ErrorRateCalculationPass(arch));
}
