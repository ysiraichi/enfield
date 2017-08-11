#include "enfield/Transform/IdentityMappingFinder.h"

efd::QbitAllocator::Mapping
efd::IdentityMappingFinder::find(ArchGraph::sRef agraph, DepsSet& deps) {
    unsigned qbits = agraph->size();
    Mapping mapping(qbits, 0);

    for (unsigned i = 0; i < qbits; ++i)
        mapping[i] = i;

    return mapping;
}

efd::IdentityMappingFinder::uRef efd::IdentityMappingFinder::Create() {
    return uRef(new IdentityMappingFinder());
}
