#include "enfield/Transform/Allocators/Simple/RandomMappingFinder.h"

#include <ctime>
#include <algorithm>
#include <chrono>
#include <random>

efd::Opt<uint32_t> Seed ("seed", "Seed to be used in random algorithms.",
std::chrono::system_clock::now().time_since_epoch().count(), false);
efd::Stat<uint32_t> SeedStat
("seed", "Seed used in the random allocator.");

int rnd(int i) {
    static std::default_random_engine generator(Seed.getVal());
    static std::uniform_int_distribution<int> distribution(0, i - 1);
    return distribution(generator);
}

efd::Mapping
efd::RandomMappingFinder::find(ArchGraph::Ref g, DepsVector& deps) {
    uint32_t qbits = g->size();
    Mapping mapping(qbits);

    for (uint32_t i = 0; i < qbits; ++i) {
        mapping[i] = i;
    }

    // "Generating" the initial mapping.
    SeedStat = Seed.getVal();
    std::random_shuffle(mapping.begin(), mapping.end(), rnd);

    return mapping;
}

efd::RandomMappingFinder::uRef efd::RandomMappingFinder::Create() {
    return uRef(new RandomMappingFinder());
}
