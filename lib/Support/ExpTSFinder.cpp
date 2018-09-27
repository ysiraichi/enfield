#include "enfield/Support/ExpTSFinder.h"
#include "enfield/Support/Defs.h"

#include <algorithm>
#include <queue>

void efd::ExpTSFinder::genAllAssigns(uint32_t n) {
    efd::InverseMap inv(n, 0);
    for (uint32_t i = 0; i < n; ++i) inv[i] = i;

    mInverseMaps.clear();

    do {
        mInverseMaps.push_back(inv);
    } while (std::next_permutation(inv.begin(), inv.end()));
}

uint32_t efd::ExpTSFinder::getTargetId(const InverseMap& source,
                                       const InverseMap& target) {
    EfdAbortIf(source.size() != target.size(),
               "The assignment map must be of same size: `" << source.size()
               << "` and `" << target.size() << "`.");

    int size = source.size();

    InverseMap translator(size, 0);
    InverseMap realtgt(size, 0);

    for (int i = 0; i < size; ++i) {
        translator[source[i]] = i;
    }

    for (int i = 0; i < size; ++i) {
        realtgt[i] = translator[target[i]];
    }

    return mMapId[realtgt];
}

// Pre-process the architechture graph, calculating the optimal swaps from every
// permutation.
void efd::ExpTSFinder::preprocess() {
    uint32_t size = mG->size();
    genAllAssigns(size);

    mMapId.clear();
    for (uint32_t i = 0, e = mInverseMaps.size(); i < e; ++i) {
        mMapId.insert(std::make_pair(mInverseMaps[i], i));
    }

    std::vector<bool> inserted(mInverseMaps.size(), false);
    mSwaps.assign(mInverseMaps.size(), SwapSeq());

    std::vector<uint32_t> cur;
    std::queue<uint32_t> q;

    // Initial permutation [0, 1, 2, 3, 4]
    q.push(0);
    inserted[0] = true;
    while (!q.empty()) {
        auto aId = q.front();
        q.pop();

        auto cur = mInverseMaps[aId];

        for (uint32_t u = 0; u < size; ++u) {
            for (uint32_t v : mG->adj(u)) {
                auto copy = cur;
                std::swap(copy[u], copy[v]);

                int cId = mMapId[copy];
                if (!inserted[cId]) {
                    inserted[cId] = true;
                    mSwaps[cId] = mSwaps[aId];
                    mSwaps[cId].push_back(Swap { u, v });
                    q.push(cId);
                }
            }
        }
    }
}

efd::SwapSeq efd::ExpTSFinder::findImpl(const InverseMap& from, const InverseMap& to) {
    return mSwaps[getTargetId(from, to)];
}

efd::ExpTSFinder::uRef efd::ExpTSFinder::Create() {
    return uRef(new ExpTSFinder());
}
