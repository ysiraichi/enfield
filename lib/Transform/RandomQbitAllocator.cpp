#include "enfield/Transform/RandomQbitAllocator.h"
#include "enfield/Support/BFSPathFinder.h"
#include "enfield/Arch/ArchGraph.h"

#include <algorithm>
#include <ctime>

efd::Opt<unsigned> Seed
("seed", "Seed to be used in the RandomQbitAllocator.", std::time(0), false);
efd::Stat<unsigned> SeedStat
("seed", "Seed used in the random allocator.");

int rnd(int i) { int r = std::rand() % i; return r; }

efd::RandomQbitAllocator::RandomQbitAllocator(ArchGraph::sRef agraph)
    : QbitAllocator(agraph) {
}

efd::RandomQbitAllocator::Mapping efd::RandomQbitAllocator::solveDependencies
(DepsSet& deps) {
    unsigned nlQbits = getNumQbits();

    Mapping mapping(nlQbits);
    for (unsigned i = 0; i < nlQbits; ++i) {
        mapping[i] = i;
    }

    // "Generating" the initial mapping.
    SeedStat = Seed.getVal();
    std::srand(Seed.getVal());
    std::random_shuffle(mapping.begin(), mapping.end(), rnd);

    Mapping assign(mArchGraph->size(), -1);
    for (unsigned i = 0; i < nlQbits; ++i) {
        assign[mapping[i]] = i;
    }

    auto finder = BFSPathFinder::Create(mArchGraph);

    // Inserting swaps.
    Mapping copy = mapping;
    for (auto& dependencies : deps) {
        auto dep = dependencies.mDeps[0];

        unsigned u = mapping[dep.mFrom], v = mapping[dep.mTo];

        if (mArchGraph->hasEdge(u, v)) continue;
        if (mArchGraph->isReverseEdge(u, v)) {
            TotalCost += RevCost.getVal();
            continue;
        }

        auto path = finder->find(u, v);
        for (auto i = path.size() - 2; i >= 1; --i) {
            unsigned u = path[i], v = path[i+1];

            if (mArchGraph->isReverseEdge(u, v))
                std::swap(u, v);

            unsigned a = assign[u], b = assign[v];

            TotalCost += SwapCost.getVal();
            insertSwapBefore(dependencies, a, b);

            std::swap(copy[a], copy[b]);
            std::swap(assign[u], assign[v]);
        }
    }

    return mapping;
}

efd::RandomQbitAllocator::uRef efd::RandomQbitAllocator::Create
(ArchGraph::sRef agraph) {
    return uRef(new RandomQbitAllocator(agraph));
}
