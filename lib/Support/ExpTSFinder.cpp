#include "enfield/Support/ExpTSFinder.h"
#include "enfield/Support/Defs.h"

#include <algorithm>
#include <queue>

void efd::ExpTSFinder::genAllAssigns(uint32_t n) {
    efd::Assign assign(n, 0);
    for (uint32_t i = 0; i < n; ++i) assign[i] = i;

    mAssigns.clear();

    do {
        mAssigns.push_back(assign);
    } while (std::next_permutation(assign.begin(), assign.end()));
}

uint32_t efd::ExpTSFinder::getTargetId(const Assign& source,
                                       const Assign& target) {
    if (source.size() != target.size()) {
        ERR << "The assignment map must be of same size: `"
            << source.size() << "` and `" << target.size() << "`." << std::endl;
        ExitWith(ExitCode::EXIT_unreachable);
    }

    int size = source.size();

    Assign translator(size, 0);
    Assign realtgt(size, 0);

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
    for (uint32_t i = 0, e = mAssigns.size(); i < e; ++i) {
        mMapId.insert(std::make_pair(mAssigns[i], i));
    }

    std::vector<bool> inserted(mAssigns.size(), false);
    mSwaps.assign(mAssigns.size(), SwapSeq());

    std::vector<uint32_t> cur;
    std::queue<uint32_t> q;

    // Initial permutation [0, 1, 2, 3, 4]
    q.push(0);
    inserted[0] = true;
    while (!q.empty()) {
        auto aId = q.front();
        q.pop();

        auto cur = mAssigns[aId];

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

efd::SwapSeq efd::ExpTSFinder::findImpl(const Assign& from, const Assign& to) {
    return mSwaps[getTargetId(from, to)];
}

efd::ExpTSFinder::uRef efd::ExpTSFinder::Create() {
    return uRef(new ExpTSFinder());
}
