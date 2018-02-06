#include "enfield/Transform/DependencyGraphBuilderPass.h"
#include "enfield/Transform/DependencyBuilderPass.h"
#include "enfield/Transform/PassCache.h"

using namespace efd;

uint8_t DependencyGraphBuilderPass::ID = 0;

DependencyGraphBuilderPass::uRef DependencyGraphBuilderPass::Create() {
    return uRef(new DependencyGraphBuilderPass());
}

bool DependencyGraphBuilderPass::run(QModule* qmod) {
    auto depbuilderPass = PassCache::Get<DependencyBuilderWrapperPass>(qmod);
    auto depbuilder = depbuilderPass->getData();
    auto& dependencies = depbuilder.getDependencies();

    uint32_t qubits = depbuilder.mXbitToNumber.getQSize();
    mData.reset(DependencyGraph::Create(qubits, Graph::Directed).release());

    for (auto& ideps : dependencies) {
        for (auto dep : ideps) {
            uint32_t a = dep.mFrom, b = dep.mTo;

            if (!mData->hasEdge(a, b)) {
                mData->putEdge(a, b, 0);
            }

            uint32_t old = mData->getW(a, b);
            mData->setW(a, b, old + 1);
        }
    }

    return false;
}
