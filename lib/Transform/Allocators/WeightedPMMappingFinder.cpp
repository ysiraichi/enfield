#include "enfield/Transform/Allocators/WeightedPMMappingFinder.h"
#include "enfield/Support/WeightedPMFinder.h"
#include "enfield/Support/uRefCast.h"

efd::WeightedPMMappingFinder::Mapping
efd::WeightedPMMappingFinder::find(ArchGraph::Ref g, DepsSet& deps) {
    uint32_t qbits = g->size();

    if (mPMFinder.get() == nullptr)
        mPMFinder = WeightedPMFinder<WeightTy>::Create();

    auto wg = toShared(WeightedGraph<WeightTy>::Create(qbits));

    std::map<std::pair<uint32_t, uint32_t>, WeightTy> wMap;
    for (auto& dep : deps) {
        Dep d = dep.mDeps[0];

        auto pair = std::make_pair(d.mFrom, d.mTo);
        if (wMap.find(pair) == wMap.end()) wMap[pair] = 0;
        ++wMap[pair];
    }

    for (auto& pair : wMap) {
        wg->putEdge(pair.first.first, pair.first.second, pair.second);
    }

    return mPMFinder->find(g, wg.get());
}

efd::WeightedPMMappingFinder::uRef efd::WeightedPMMappingFinder::Create() {
    return uRef(new WeightedPMMappingFinder());
}
