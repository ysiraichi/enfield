#include "enfield/Transform/Allocators/Simple/WeightedSIMappingFinder.h"
#include "enfield/Support/WeightedSIFinder.h"
#include "enfield/Support/uRefCast.h"

efd::Mapping
efd::WeightedSIMappingFinder::find(ArchGraph::Ref g, DepsVector& deps) {
    uint32_t qbits = g->size();

    if (mSIFinder.get() == nullptr)
        mSIFinder = WeightedSIFinder<WeightTy>::Create();

    auto wg = toShared(WeightedGraph<WeightTy>::Create(qbits, Graph::Directed));

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

    return mSIFinder->find(g, wg.get()).m;
}

efd::WeightedSIMappingFinder::uRef efd::WeightedSIMappingFinder::Create() {
    return uRef(new WeightedSIMappingFinder());
}
