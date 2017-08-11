#include "enfield/Transform/WeightedPMMappingFinder.h"
#include "enfield/Support/WeightedPMFinder.h"
#include "enfield/Support/uRefCast.h"

efd::WeightedPMMappingFinder::Mapping
efd::WeightedPMMappingFinder::find(ArchGraph::sRef agraph, DepsSet& deps) {
    unsigned qbits = agraph->size();

    if (mPMFinder.get() == nullptr)
        mPMFinder = WeightedPMFinder<WeightTy>::Create(agraph);

    auto wg = toShared(WeightedGraph<WeightTy>::Create(qbits));

    std::map<std::pair<unsigned, unsigned>, WeightTy> wMap;
    for (auto& dep : deps) {
        Dep d = dep.mDeps[0];

        auto pair = std::make_pair(d.mFrom, d.mTo);
        if (wMap.find(pair) == wMap.end()) wMap[pair] = 0;
        ++wMap[pair];
    }

    for (auto& pair : wMap) {
        wg->putEdge(pair.first.first, pair.first.second, pair.second);
    }

    return mPMFinder->find(wg.get());
}

efd::WeightedPMMappingFinder::uRef efd::WeightedPMMappingFinder::Create() {
    return uRef(new WeightedPMMappingFinder());
}
