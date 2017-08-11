#include "enfield/Transform/RandomMappingFinder.h"

#include <ctime>
#include <algorithm>

efd::Opt<unsigned> Seed
("seed", "Seed to be used in the RandomQbitAllocator.", std::time(0), false);
efd::Stat<unsigned> SeedStat
("seed", "Seed used in the random allocator.");

int rnd(int i) { int r = std::rand() % i; return r; }

efd::MappingFinder::Mapping
efd::RandomMappingFinder::find(ArchGraph::sRef agraph, DepsSet& deps) {
    unsigned qbits = agraph->size();
    Mapping mapping(qbits);

    for (unsigned i = 0; i < qbits; ++i) {
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
