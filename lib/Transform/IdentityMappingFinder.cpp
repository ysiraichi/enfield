#include "enfield/Transform/IdentityMappingFinder.h"

efd::QbitAllocator::Mapping
efd::IdentityMappingFinder::find(ArchGraph::Ref g, DepsSet& deps) {
    uint32_t qbits = g->size();
    Mapping mapping(qbits, 0);

    for (uint32_t i = 0; i < qbits; ++i)
        mapping[i] = i;

    return mapping;
}

efd::IdentityMappingFinder::uRef efd::IdentityMappingFinder::Create() {
    return uRef(new IdentityMappingFinder());
}
