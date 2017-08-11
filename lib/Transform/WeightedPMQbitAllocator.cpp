#include "enfield/Transform/WeightedPMQbitAllocator.h"
#include "enfield/Support/BFSPathFinder.h"

#include <cassert>
#include <map>

efd::WeightedPMQbitAllocator::WeightedPMQbitAllocator(ArchGraph::sRef agraph) :
    QbitAllocator(agraph), mWG(nullptr), mPMFinder(nullptr) {
}

std::unique_ptr<efd::WeightedGraph<efd::WeightedPMQbitAllocator::WeightTy>>
efd::WeightedPMQbitAllocator::createWG(DepsSet& deps) {
    auto wg = WeightedGraph<WeightTy>::Create(QbitAllocator::getNumQbits());

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

    return wg;
}

std::vector<unsigned>
efd::WeightedPMQbitAllocator::genAssign(std::vector<unsigned> match) {
    unsigned size = mArchGraph->size();
    std::vector<unsigned> assign(size, size);

    // for 'u' in arch; and 'a' in prog:
    // if 'a' -> 'u', then 'u' -> 'a'
    for (unsigned i = 0, e = match.size(); i < e; ++i)
        assign[match[i]] = i;

    // Fill the qubits in the architecture that were not mapped.
    unsigned id = match.size();
    for (unsigned i = 0; i < size; ++i)
        assign[i] = (assign[i] == size) ? id++ : assign[i];

    return assign;
}

efd::QbitAllocator::Mapping
efd::WeightedPMQbitAllocator::solveDependencies(DepsSet& deps) {
    for (auto& dep : deps)
        assert(dep.getSize() == 1 && "More than one dependency per instruction.");

    const unsigned SWAP_COST = SwapCost.getVal();
    const unsigned REV_COST = RevCost.getVal();

    mWG = createWG(deps);

    if (mPMFinder.get() == nullptr)
        mPMFinder = WeightedPMFinder<WeightTy>::Create(mArchGraph);
    if (mPathFinder.get() == nullptr)
        mPathFinder = BFSPathFinder::Create(mArchGraph);

    Mapping initial = mPMFinder->find(mWG.get());
    Mapping match = initial;

    for (auto& dep : deps) {
        Dep d = dep.mDeps[0];

        // Program qubits (a, b)
        unsigned a = d.mFrom, b = d.mTo;

        // Physical qubits (u, v)
        unsigned u = match[a], v = match[b];

        if (mArchGraph->hasEdge(u, v)) continue;
        if (mArchGraph->isReverseEdge(u, v)) {
            TotalCost += REV_COST;
            continue;
        }

        auto assign = genAssign(match);
        auto path = mPathFinder->find(u, v);

        // It should stop before swapping the 'source' qubit.
        for (auto i = path.size() - 2; i >= 1; --i) {
            unsigned u = path[i], v = path[i+1];

            if (mArchGraph->isReverseEdge(u, v))
                std::swap(u, v);

            unsigned a = assign[u], b = assign[v];

            TotalCost += SWAP_COST;
            insertSwapBefore(dep, a, b);

            std::swap(match[a], match[b]);
            std::swap(assign[u], assign[v]);
        }
    }

    return initial;
}

void efd::WeightedPMQbitAllocator::setWPMFinder(WeightedPMFinder<WeightTy>::sRef finder) {
    mPMFinder = finder;
}

void efd::WeightedPMQbitAllocator::setPathFinder(PathFinder::sRef finder) {
    mPathFinder = finder;
}

efd::WeightedPMQbitAllocator::uRef
efd::WeightedPMQbitAllocator::Create(ArchGraph::sRef agraph) {
    return uRef(new WeightedPMQbitAllocator(agraph));
}
