#include "enfield/Transform/QbitterQbitAllocator.h"
#include "enfield/Support/CommandLine.h"

#include <unordered_map>
#include <limits>
#include <queue>
#include <iostream>
#include <algorithm>

const unsigned LcxCost = 10;

efd::QbitAllocator::Mapping efd::QbitterQbitAllocator::solveDependencies(DepsSet& deps) {
    Mapping mapping;

    // Start with a naive mapping.
    // Prog -> Arch
    // 0 -> 0
    // 1 -> 1
    // 2 -> 2
    // 3 -> 3
    // 4 -> 4
    for (unsigned i = 0, e = getNumQbits(); i < e; ++i) {
        mapping.push_back(i);
    }

    for (auto& lineDeps : deps) {
        auto dep = lineDeps[0];

        // (u, v) edge in Arch
        unsigned u = mapping[dep.mFrom], v = mapping[dep.mTo];

        if (mArchGraph->hasEdge(u, v))
            continue;

        if (mArchGraph->isReverseEdge(u, v)) {
            TotalCost += RevCost.getVal();
            continue;
        }

        replaceByLCNOT(lineDeps, u, 2, v);
        TotalCost += LcxCost;
    }

    return mapping;
}

efd::QbitterQbitAllocator::QbitterQbitAllocator(ArchGraph::sRef pGraph) 
    : QbitAllocator(pGraph) {
}

efd::QbitterQbitAllocator::uRef efd::QbitterQbitAllocator::Create
(ArchGraph::sRef archGraph) {
    return uRef(new QbitterQbitAllocator(archGraph));
}
