#include "enfield/Transform/RandomMappingFinder.h"

#include <ctime>
#include <algorithm>

efd::Opt<uint32_t> Seed
("seed", "Seed to be used in the RandomQbitAllocator.", std::time(0), false);
efd::Stat<uint32_t> SeedStat
("seed", "Seed used in the random allocator.");

int rnd(int i) { int r = std::rand() % i; return r; }

efd::MappingFinder::Mapping
efd::RandomMappingFinder::find(ArchGraph::Ref g, DepsSet& deps) {
    uint32_t qbits = g->size();
    Mapping mapping(qbits);

    for (uint32_t i = 0; i < qbits; ++i) {
        mapping[i] = i;
    }

    // "Generating" the initial mapping.
    SeedStat = Seed.getVal();
    std::srand(Seed.getVal());
    std::random_shuffle(mapping.begin(), mapping.end(), rnd);

    return mapping;
}

efd::RandomMappingFinder::uRef efd::RandomMappingFinder::Create() {
    return uRef(new RandomMappingFinder());
}
