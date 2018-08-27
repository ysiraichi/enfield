#include "enfield/Transform/QModuleQualityEvalPass.h"
#include "enfield/Transform/LayersBuilderPass.h"
#include "enfield/Transform/PassCache.h"
#include "enfield/Transform/Utils.h"
#include "enfield/Support/CommandLine.h"
#include "enfield/Support/Defs.h"

using namespace efd;
uint8_t QModuleQualityEvalPass::ID = 0;

QModuleQualityEvalPass::QModuleQualityEvalPass(MapGateUInt gatesW)
    : mGatesW(gatesW) {}

bool QModuleQualityEvalPass::run(QModule::Ref qmod) {
     MapGateUInt gatesQ;

    auto &layers = PassCache::Get<LayersBuilderPass>(qmod)->getData();

    for (auto it = qmod->stmt_begin(), e = qmod->stmt_end(); it != e; ++it) {
        auto qopNode = GetStatementPair(it->get()).second;
        auto nodeOp = qopNode->getOperation();

        if (gatesQ.find(nodeOp) == gatesQ.end()) {
            gatesQ[nodeOp] = 0;
        }

        ++gatesQ[nodeOp];
    }

    uint32_t totalWCost = 0;
    for (auto& pair : gatesQ) {
        if (mGatesW.find(pair.first) != mGatesW.end()) {
            totalWCost += (pair.second * mGatesW.at(pair.first));
        } else {
            WAR << "No weights for gate: `" << pair.first << "`." << std::endl;
        }
    }

    mData.mGates = qmod->getNumberOfStmts();
    mData.mDepth = layers.size();
    mData.mWeightedCost = totalWCost;

    return false;
}

QModuleQualityEvalPass::uRef QModuleQualityEvalPass::Create(MapGateUInt gatesW) {
    return uRef(new QModuleQualityEvalPass(gatesW));
}
